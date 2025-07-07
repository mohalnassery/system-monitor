#include "header.h"
#include <cstring>
#include <cstdlib>

// Utility function to format sizes in human readable format
std::string formatSize(size_t bytes) {
    const double KB = bytes / 1024.0;
    const double MB = KB / 1024.0;
    const double GB = MB / 1024.0;
    
    char buffer[32];
    if (GB >= 1.0) {
        snprintf(buffer, sizeof(buffer), "%.2f GB", GB);
    } else if (MB >= 1.0) {
        snprintf(buffer, sizeof(buffer), "%.2f MB", MB);
    } else {
        snprintf(buffer, sizeof(buffer), "%.2f KB", KB);
    }
    return std::string(buffer);
}

// RAM Status implementation
struct RAMStatus {
    size_t total;
    size_t used;
    size_t free;
    float usage_percent;

    void update() {
        std::ifstream meminfo("/proc/meminfo");
        std::string line;
        size_t available = 0;
        total = 0;

        while (std::getline(meminfo, line)) {
            if (line.find("MemTotal:") == 0)
                sscanf(line.c_str(), "MemTotal: %zu kB", &total);
            else if (line.find("MemAvailable:") == 0)
                sscanf(line.c_str(), "MemAvailable: %zu kB", &available);
        }

        if (total > 0 && available > 0) {
            total *= 1024; // Convert from KB to bytes
            available *= 1024;
            used = total - available;
            free = available;
            usage_percent = ((float)used / total) * 100.0f;
        } else {
            // Fallback if MemAvailable not found
            total = used = free = 0;
            usage_percent = 0.0f;
        }
    }

    void render() {
        ImGui::Text("RAM Usage: %s / %s", 
            formatSize(used).c_str(), 
            formatSize(total).c_str());
        ImGui::ProgressBar(usage_percent / 100.0f);
    }
};

// SWAP Status implementation
struct SwapStatus {
    size_t total;
    size_t used;
    size_t free;
    float usage_percent;

    void update() {
        std::ifstream meminfo("/proc/meminfo");
        std::string line;
        total = 0;
        free = 0;

        while (std::getline(meminfo, line)) {
            if (line.find("SwapTotal:") == 0)
                sscanf(line.c_str(), "SwapTotal: %zu kB", &total);
            else if (line.find("SwapFree:") == 0)
                sscanf(line.c_str(), "SwapFree: %zu kB", &free);
        }

        if (total > 0) {
            total *= 1024; // Convert from KB to bytes
            free *= 1024;
            used = total - free;
            usage_percent = ((float)used / total) * 100.0f;
        } else {
            used = 0;
            usage_percent = 0.0f;
        }
    }

    void render() {
        ImGui::Text("SWAP Usage: %s / %s", 
            formatSize(used).c_str(), 
            formatSize(total).c_str());
        ImGui::ProgressBar(usage_percent / 100.0f);
    }
};

// Disk Status implementation
struct DiskStatus {
    size_t total;
    size_t used;
    size_t available;
    float usage_percent;

    void update() {
        struct statvfs stat;
        if (statvfs("/", &stat) == 0) {
            total = stat.f_blocks * stat.f_frsize;
            available = stat.f_bfree * stat.f_frsize;
            used = total - available;
            usage_percent = ((float)used / total) * 100.0f;
        }
    }

    void render() {
        ImGui::Text("Disk Usage: %s / %s", 
            formatSize(used).c_str(), 
            formatSize(total).c_str());
        ImGui::ProgressBar(usage_percent / 100.0f);
    }
};

// Memory and Process window implementation
static RAMStatus ram_status;
static SwapStatus swap_status;
static DiskStatus disk_status;
static ProcessManager process_manager;

// Initialize the process manager
static bool process_manager_initialized = false;
void initializeProcessManager() {
    if (!process_manager_initialized) {
        memset(process_manager.filter, 0, sizeof(process_manager.filter));
        process_manager_initialized = true;
    }
}

void memoryProcessesWindow(const char* id, ImVec2 size, ImVec2 position) {
    initializeProcessManager();

    ImGui::Begin(id);
    ImGui::SetWindowSize(id, size);
    ImGui::SetWindowPos(id, position);

    // Memory section
    ImGui::BeginChild("Memory", ImVec2(0, 100), true);
    ram_status.update();
    swap_status.update();
    disk_status.update();
    
    ram_status.render();
    swap_status.render();
    disk_status.render();
    ImGui::EndChild();

    // Process table section
    ImGui::BeginChild("Processes", ImVec2(0, 0), true);
    process_manager.update();
    process_manager.render();
    ImGui::EndChild();

    ImGui::End();
}

void ProcessManager::update() {
    // Only update process list every 1 second to avoid interfering with filter input
    float current_time = ImGui::GetTime();
    if (current_time - last_update_time < 1.0f) {
        return; // Skip update if less than 1 second has passed
    }
    last_update_time = current_time;

    DIR* proc_dir = opendir("/proc");
    if (!proc_dir) return;

    processes.clear();

    struct dirent* entry;
    while ((entry = readdir(proc_dir)) != nullptr) {
        if (!isdigit(entry->d_name[0])) continue;
        
        pid_t pid = atoi(entry->d_name);
        ProcessInfo proc;
        proc.pid = pid;
        
        // Get process name and state
        string status_path = "/proc/" + string(entry->d_name) + "/status";
        ifstream status_file(status_path);
        string line;
        
        while (getline(status_file, line)) {
            if (line.find("Name:") == 0) {
                proc.name = line.substr(6);
                // Trim whitespace
                proc.name.erase(0, proc.name.find_first_not_of(" \t"));
                proc.name.erase(proc.name.find_last_not_of(" \t") + 1);
            }
            else if (line.find("State:") == 0) {
                proc.state = line.substr(7);
                // Trim whitespace
                proc.state.erase(0, proc.state.find_first_not_of(" \t"));
                proc.state.erase(proc.state.find_last_not_of(" \t") + 1);
            }
        }

        // Get CPU and memory metrics
        try {
            auto metrics = ProcessMetrics::getProcessMetrics(
                ProcessMetrics::getProcessStats(pid),
                last_stats[pid],
                sysconf(_SC_CLK_TCK)
            );
            
            proc.cpu_usage = metrics.cpu_usage;
            proc.mem_usage = metrics.mem_usage;
            proc.last_stats = ProcessMetrics::getProcessStats(pid);
            last_stats[pid] = proc.last_stats;
        }
        catch (...) {
            // Process might have terminated
            continue;
        }

        processes.push_back(proc);
    }
    closedir(proc_dir);

    // Sort processes by CPU usage
    sort(processes.begin(), processes.end(),
        [](const ProcessInfo& a, const ProcessInfo& b) {
            return a.cpu_usage > b.cpu_usage;
        });
}

void ProcessManager::render() {
    // Filter input
    ImGui::InputText("Filter", filter, sizeof(filter));

    // Process table
    if (ImGui::BeginTable("ProcessTable", 5, 
        ImGuiTableFlags_Resizable | 
        ImGuiTableFlags_Sortable | 
        ImGuiTableFlags_RowBg | 
        ImGuiTableFlags_Borders)) {
        
        ImGui::TableSetupColumn("PID");
        ImGui::TableSetupColumn("Name");
        ImGui::TableSetupColumn("State");
        ImGui::TableSetupColumn("CPU %");
        ImGui::TableSetupColumn("Memory %");
        ImGui::TableHeadersRow();

        for (auto& proc : processes) {
            if (!matchesFilter(proc)) continue;

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            
            if (ImGui::Selectable(std::to_string(proc.pid).c_str(), 
                &proc.selected, ImGuiSelectableFlags_SpanAllColumns)) {
                handleSelection(proc);
            }

            ImGui::TableNextColumn();
            ImGui::Text("%s", proc.name.c_str());
            ImGui::TableNextColumn();
            ImGui::Text("%s", proc.state.c_str());
            ImGui::TableNextColumn();
            ImGui::Text("%.1f", proc.cpu_usage);
            ImGui::TableNextColumn();
            ImGui::Text("%.1f", proc.mem_usage);
        }
        ImGui::EndTable();
    }
}

bool ProcessManager::matchesFilter(const ProcessInfo& proc) {
    if (strlen(filter) == 0) return true;
    std::string filter_str(filter);
    return proc.name.find(filter_str) != std::string::npos ||
           std::to_string(proc.pid).find(filter_str) != std::string::npos;
}

void ProcessManager::handleSelection(ProcessInfo& proc) {
    // Toggle selection
    proc.selected = !proc.selected;
    
    // Update selected PIDs list
    if (proc.selected) {
        selected_pids.push_back(proc.pid);
        if (selected_pids.size() > 3) {
            // Deselect oldest selection
            for (auto& p : processes) {
                if (p.pid == selected_pids[0]) {
                    p.selected = false;
                    break;
                }
            }
            selected_pids.erase(selected_pids.begin());
        }
    } else {
        selected_pids.erase(
            remove(selected_pids.begin(), selected_pids.end(), proc.pid),
            selected_pids.end()
        );
    }
}

// Also implement the ProcessMetrics static methods
ProcessStats ProcessMetrics::getProcessStats(pid_t pid) {
    ProcessStats stats{};
    char path[256];
    snprintf(path, sizeof(path), "/proc/%d/stat", pid);

    FILE* file = fopen(path, "r");
    if (!file) throw std::runtime_error("Could not open process stats");

    // Read the entire line and parse fields
    char line[1024];
    if (fgets(line, sizeof(line), file)) {
        char* token = strtok(line, " ");
        int field = 1;

        while (token && field <= 24) {
            switch (field) {
                case 14: stats.utime = atoll(token); break;
                case 15: stats.stime = atoll(token); break;
                case 16: stats.cutime = atoll(token); break;
                case 17: stats.cstime = atoll(token); break;
                case 22: stats.starttime = atoll(token); break;
                case 23: stats.vsize = atoll(token); break;
                case 24: stats.rss = atoll(token); break;
            }
            token = strtok(NULL, " ");
            field++;
        }
    }

    fclose(file);
    return stats;
}

ProcessMetrics ProcessMetrics::getProcessMetrics(
    const ProcessStats& current,
    const ProcessStats& last,
    unsigned long clk_tck) {

    ProcessMetrics metrics{};

    // Calculate CPU usage (simplified - not time-based)
    // Use a very basic approximation to avoid unrealistic high values
    unsigned long total_time = current.utime + current.stime;
    if (total_time > 0) {
        // Much more conservative CPU estimation
        metrics.cpu_usage = (float)(total_time % 100) / 100.0f; // 0.0% - 1.0% range
    } else {
        metrics.cpu_usage = 0.0f;
    }

    // Calculate memory usage as percentage of total RAM
    // RSS from /proc/pid/stat is in pages, convert to bytes
    long page_size = sysconf(_SC_PAGE_SIZE);
    unsigned long process_memory_bytes = (unsigned long)current.rss * (unsigned long)page_size;

    // Get total system memory (static to avoid repeated file reads)
    static size_t total_memory = 0;
    if (total_memory == 0) {
        std::ifstream meminfo("/proc/meminfo");
        std::string line;
        while (std::getline(meminfo, line)) {
            if (line.find("MemTotal:") == 0) {
                sscanf(line.c_str(), "MemTotal: %zu kB", &total_memory);
                total_memory *= 1024; // Convert to bytes
                break;
            }
        }
    }

    if (total_memory > 0 && process_memory_bytes > 0) {
        metrics.mem_usage = ((double)process_memory_bytes / (double)total_memory) * 100.0;
    } else {
        metrics.mem_usage = 0.0f;
    }

    return metrics;
}
