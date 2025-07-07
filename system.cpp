#include "header.h"
#include <sstream>
#include <algorithm>

// System information structures
struct SystemInfo {
    std::string os_type;
    std::string logged_user;
    std::string hostname;
    std::string cpu_model;
};

struct SystemProcessStats {
    int total = 0;
    int running = 0;
    int sleeping = 0;
    int zombie = 0;
    int stopped = 0;
};

struct CPUData {
    std::vector<float> usage_history;
    float current_usage = 0.0f;
    float fps = 60.0f;
    float scale = 100.0f;
    bool animate = true;
    CPUStats last_stats;
    CPUStats current_stats;
};

struct FanData {
    bool enabled = false;
    int speed = 0;
    int level = 0;
    std::vector<float> history;
    float fps = 60.0f;
    float scale = 5000.0f;
    bool animate = true;
};

struct ThermalData {
    float current_temp = 0.0f;
    std::vector<float> history;
    float fps = 60.0f;
    float scale = 100.0f;
    bool animate = true;
};

// Global data
static SystemInfo g_system_info;
static SystemProcessStats g_process_stats;
static CPUData g_cpu_data;
static FanData g_fan_data;
static ThermalData g_thermal_data;

// Helper functions
std::string readFileContent(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) return "";
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

// System information functions
void updateSystemInfo() {
    // Get OS Type from /proc/version
    std::string version = readFileContent("/proc/version");
    if (!version.empty()) {
        size_t pos = version.find("Linux");
        g_system_info.os_type = (pos != std::string::npos) ? "Linux" : "Unknown";
    }

    // Get logged user
    char* user = getlogin();
    g_system_info.logged_user = user ? user : "unknown";

    // Get hostname
    char hostname[HOST_NAME_MAX];
    if (gethostname(hostname, HOST_NAME_MAX) == 0) {
        g_system_info.hostname = hostname;
    }

    // Get CPU Model
    std::string cpuinfo = readFileContent("/proc/cpuinfo");
    size_t pos = cpuinfo.find("model name");
    if (pos != std::string::npos) {
        size_t end = cpuinfo.find("\n", pos);
        size_t start = cpuinfo.find(":", pos) + 2;
        g_system_info.cpu_model = cpuinfo.substr(start, end - start);
    }
}

// Process statistics functions
void updateProcessStats() {
    g_process_stats = SystemProcessStats(); // Reset counters
    
    DIR* proc_dir = opendir("/proc");
    if (!proc_dir) return;

    struct dirent* entry;
    while ((entry = readdir(proc_dir)) != nullptr) {
        // Skip non-process entries
        if (!isdigit(entry->d_name[0])) continue;

        std::string status_path = "/proc/" + std::string(entry->d_name) + "/status";
        std::string status = readFileContent(status_path);
        
        if (status.find("State:\tR") != std::string::npos) g_process_stats.running++;
        else if (status.find("State:\tS") != std::string::npos) g_process_stats.sleeping++;
        else if (status.find("State:\tZ") != std::string::npos) g_process_stats.zombie++;
        else if (status.find("State:\tT") != std::string::npos) g_process_stats.stopped++;
        
        g_process_stats.total++;
    }
    closedir(proc_dir);
}

// CPU usage calculation
float calculateCPUUsage() {
    std::string stat = readFileContent("/proc/stat");
    std::istringstream iss(stat);
    std::string cpu;
    CPUStats stats;
    
    iss >> cpu 
        >> stats.user >> stats.nice >> stats.system 
        >> stats.idle >> stats.iowait >> stats.irq 
        >> stats.softirq >> stats.steal >> stats.guest 
        >> stats.guestNice;

    if (cpu != "cpu") return 0.0f;

    long long int prev_idle = g_cpu_data.last_stats.idle + g_cpu_data.last_stats.iowait;
    long long int idle = stats.idle + stats.iowait;

    long long int prev_non_idle = g_cpu_data.last_stats.user + g_cpu_data.last_stats.nice + 
                                 g_cpu_data.last_stats.system + g_cpu_data.last_stats.irq +
                                 g_cpu_data.last_stats.softirq + g_cpu_data.last_stats.steal;
    
    long long int non_idle = stats.user + stats.nice + stats.system + 
                            stats.irq + stats.softirq + stats.steal;

    long long int prev_total = prev_idle + prev_non_idle;
    long long int total = idle + non_idle;

    long long int total_diff = total - prev_total;
    long long int idle_diff = idle - prev_idle;

    g_cpu_data.last_stats = stats;

    if (total_diff == 0) return 0.0f;
    return (float)(total_diff - idle_diff) * 100.0f / (float)total_diff;
}

// Update functions for graphs
void updateCPUData() {
    if (!g_cpu_data.animate) return;
    
    float usage = calculateCPUUsage();
    g_cpu_data.current_usage = usage;
    g_cpu_data.usage_history.push_back(usage);
    
    // Keep history at reasonable size
    if (g_cpu_data.usage_history.size() > 100) {
        g_cpu_data.usage_history.erase(g_cpu_data.usage_history.begin());
    }
}

void updateFanData() {
    if (!g_fan_data.animate) return;
    
    // Try to detect fan capabilities if not already checked
    static bool fan_capability_checked = false;
    static bool fan_monitoring_available = false;
    
    if (!fan_capability_checked) {
        // Check if any fan monitoring interface exists
        const std::vector<std::string> possible_paths = {
            "/sys/class/hwmon",
            "/proc/acpi/ibm/fan",
            "/sys/devices/platform/coretemp.0/hwmon",
            "/sys/class/thermal"
        };
        
        for (const auto& path : possible_paths) {
            if (access(path.c_str(), F_OK) == 0) {
                fan_monitoring_available = true;
                break;
            }
        }
        
        fan_capability_checked = true;
        
        if (!fan_monitoring_available) {
            std::cerr << "Warning: No fan monitoring interfaces detected on this system" << std::endl;
        }
    }
    
    // If no fan monitoring is available, use simulated/placeholder data
    if (!fan_monitoring_available) {
        g_fan_data.enabled = false;
        g_fan_data.speed = 0;
        g_fan_data.level = 0;
        g_fan_data.history.push_back(0.0f);
        if (g_fan_data.history.size() > 100) {
            g_fan_data.history.erase(g_fan_data.history.begin());
        }
        return;
    }
    
    bool fan_found = false;

    // Method 1: Try ThinkPad-specific fan interface
    std::string fan_data = readFileContent("/proc/acpi/ibm/fan");
    if (!fan_data.empty()) {
        std::istringstream iss(fan_data);
        std::string line;
        while (std::getline(iss, line)) {
            if (line.find("status:") != std::string::npos) {
                g_fan_data.enabled = (line.find("disabled") == std::string::npos);
            }
            else if (line.find("speed:") != std::string::npos) {
                size_t pos = line.find(":");
                if (pos != std::string::npos) {
                    std::string speed_str = line.substr(pos + 1);
                    speed_str.erase(0, speed_str.find_first_not_of(" \t\r\n"));
                    try {
                        g_fan_data.speed = std::stoi(speed_str);
                        fan_found = true;
                    } catch (...) {}
                }
            }
        }
    }

    // Method 2: Try generic hwmon interface
    if (!fan_found) {
        const std::vector<std::string> hwmon_paths = {
            "/sys/class/hwmon",
            "/sys/devices/platform/coretemp.0/hwmon",
            "/sys/devices/platform/it87.2608/hwmon",
            "/sys/devices/platform/nct6775.2592/hwmon"
        };

        for (const auto& base_path : hwmon_paths) {
            DIR* hwmon = opendir(base_path.c_str());
            if (!hwmon) continue;

            struct dirent* entry;
            while ((entry = readdir(hwmon)) != nullptr) {
                if (entry->d_name[0] == '.') continue;

                std::string device_path = base_path + "/" + entry->d_name + "/";
                
                // Try different fan input files
                const std::vector<std::string> fan_files = {
                    "fan1_input",
                    "fan2_input",
                    "fan3_input"
                };

                for (const auto& fan_file : fan_files) {
                    std::string speed_str = readFileContent(device_path + fan_file);
                    if (!speed_str.empty()) {
                        try {
                            int speed = std::stoi(speed_str);
                            if (speed >= 0) {
                                g_fan_data.speed = speed;
                                g_fan_data.enabled = (speed > 0);
                                fan_found = true;
                                break;
                            }
                        } catch (...) {}
                    }
                }

                if (fan_found) {
                    // Try to read PWM value
                    std::string pwm_str = readFileContent(device_path + "pwm1");
                    if (!pwm_str.empty()) {
                        try {
                            g_fan_data.level = std::stoi(pwm_str) * 100 / 255;
                        } catch (...) {}
                    }
                    break;
                }
            }
            closedir(hwmon);
            if (fan_found) break;
        }
    }

    // Method 3: Try thermal cooling devices (CPU throttling/fan control)
    if (!fan_found) {
        DIR* thermal = opendir("/sys/class/thermal");
        if (thermal) {
            struct dirent* entry;
            while ((entry = readdir(thermal)) != nullptr) {
                if (strncmp(entry->d_name, "cooling_device", 14) == 0) {
                    std::string path = "/sys/class/thermal/" + std::string(entry->d_name) + "/";
                    std::string type = readFileContent(path + "type");
                    std::string cur_state = readFileContent(path + "cur_state");
                    std::string max_state = readFileContent(path + "max_state");

                    // Check if it's a processor cooling device (CPU throttling)
                    if (type.find("Processor") != std::string::npos ||
                        type.find("processor") != std::string::npos ||
                        type.find("Fan") != std::string::npos ||
                        type.find("fan") != std::string::npos) {
                        try {
                            int current = std::stoi(cur_state);
                            int maximum = std::stoi(max_state);

                            // Convert cooling state to appropriate display values
                            // Higher cooling state = more active cooling
                            if (maximum > 0) {
                                g_fan_data.speed = current; // Use raw cooling state (0-3)
                                g_fan_data.level = (current * 100) / maximum;  // Percentage
                                g_fan_data.enabled = (current > 0);
                                fan_found = true;
                                break;
                            }
                        } catch (...) {}
                    }
                }
            }
            closedir(thermal);
        }
    }

    // If no fan data found, mark as inactive
    if (!fan_found) {
        g_fan_data.enabled = false;
        g_fan_data.speed = 0;
        g_fan_data.level = 0;
    }

    // Update history
    g_fan_data.history.push_back(static_cast<float>(g_fan_data.speed));
    if (g_fan_data.history.size() > 100) {
        g_fan_data.history.erase(g_fan_data.history.begin());
    }

  }

void updateThermalData() {
    if (!g_thermal_data.animate) return;

    // Try reading from /sys/class/thermal first
    std::string temp_str = readFileContent("/sys/class/thermal/thermal_zone0/temp");
    if (!temp_str.empty()) {
        g_thermal_data.current_temp = std::stof(temp_str) / 1000.0f; // Convert from millicelsius
    } else {
        // Fallback to /proc/acpi/ibm/thermal
        temp_str = readFileContent("/proc/acpi/ibm/thermal");
        if (!temp_str.empty()) {
            std::istringstream iss(temp_str);
            std::string temperatures;
            iss >> temperatures;
            g_thermal_data.current_temp = std::stof(temperatures);
        }
    }

    g_thermal_data.history.push_back(g_thermal_data.current_temp);
    if (g_thermal_data.history.size() > 100) {
        g_thermal_data.history.erase(g_thermal_data.history.begin());
    }
}

// Render functions
void renderSystemInfo() {
    ImGui::Text("OS Type: %s", g_system_info.os_type.c_str());
    ImGui::Text("User: %s", g_system_info.logged_user.c_str());
    ImGui::Text("Hostname: %s", g_system_info.hostname.c_str());
    ImGui::Text("CPU Model: %s", g_system_info.cpu_model.c_str());
    
    ImGui::Separator();
    
    ImGui::Text("Processes:");
    ImGui::Text("Total: %d", g_process_stats.total);
    ImGui::Text("Running: %d", g_process_stats.running);
    ImGui::Text("Sleeping: %d", g_process_stats.sleeping);
    ImGui::Text("Zombie: %d", g_process_stats.zombie);
    ImGui::Text("Stopped: %d", g_process_stats.stopped);
}

void renderCPUTab() {
    // FPS Slider
    ImGui::SliderFloat("FPS##cpu", &g_cpu_data.fps, 1.0f, 60.0f);
    
    // Scale Slider
    ImGui::SliderFloat("Scale##cpu", &g_cpu_data.scale, 0.0f, 100.0f);
    
    // Animation Toggle
    ImGui::Checkbox("Animate##cpu", &g_cpu_data.animate);
    
    // CPU Usage Graph
    ImGui::PlotLines("CPU Usage", 
        g_cpu_data.usage_history.data(), 
        g_cpu_data.usage_history.size(),
        0, 
        ("CPU: " + std::to_string(g_cpu_data.current_usage) + "%").c_str(),
        0.0f, 
        g_cpu_data.scale,
        ImVec2(0, 80));
}

void renderFanTab() {
    // FPS Slider
    ImGui::SliderFloat("FPS##fan", &g_fan_data.fps, 1.0f, 60.0f);

    // Scale Slider - adjust range based on monitoring type
    static bool has_cooling_device = (access("/sys/class/thermal/cooling_device0", F_OK) == 0);
    static bool has_direct_fan = (access("/proc/acpi/ibm/fan", F_OK) == 0);

    float max_scale = (has_cooling_device && !has_direct_fan) ? 10.0f : 5000.0f;
    ImGui::SliderFloat("Scale##fan", &g_fan_data.scale, 0.0f, max_scale);

    // Animation Toggle
    ImGui::Checkbox("Animate##fan", &g_fan_data.animate);

    ImGui::Spacing();

    // Check what type of cooling/fan data we have
    static bool has_hwmon_fan = false;

    // Check for hwmon fan files
    if (!has_hwmon_fan) {
        DIR* hwmon = opendir("/sys/class/hwmon");
        if (hwmon) {
            struct dirent* entry;
            while ((entry = readdir(hwmon)) != nullptr) {
                if (entry->d_name[0] == '.') continue;
                std::string device_path = "/sys/class/hwmon/" + std::string(entry->d_name) + "/fan1_input";
                if (access(device_path.c_str(), F_OK) == 0) {
                    has_hwmon_fan = true;
                    break;
                }
            }
            closedir(hwmon);
        }
    }

    // Display appropriate status message
    if (has_direct_fan) {
        ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "✓ Direct fan monitoring available");
    } else if (has_hwmon_fan) {
        ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "✓ Hardware monitoring fan detected");
    } else if (has_cooling_device) {
        ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "⚠ Using thermal cooling device");
        ImGui::TextWrapped("No direct fan monitoring available. Showing CPU thermal throttling state instead.");
    } else {
        ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), "⚠ No fan monitoring available");
        ImGui::TextWrapped("This system doesn't expose fan information through standard interfaces.");
    }

    ImGui::Spacing();

    // Status information
    if (has_cooling_device && !has_direct_fan && !has_hwmon_fan) {
        ImGui::Text("Cooling Status: %s", g_fan_data.enabled ? "Active" : "Inactive");
        ImGui::Text("Throttle Level: %d%%", g_fan_data.level);
        ImGui::Text("Cooling State: %d", g_fan_data.speed);
    } else {
        ImGui::Text("Fan Status: %s", g_fan_data.enabled ? "Active" : "Inactive");
        ImGui::Text("Speed: %d RPM", g_fan_data.speed);
        ImGui::Text("Level: %d%%", g_fan_data.level);
    }

    // Fan/Cooling Graph
    const char* graph_title = (has_cooling_device && !has_direct_fan && !has_hwmon_fan) ?
                             "Thermal Cooling" : "Fan Speed";
    const char* overlay_text = (has_cooling_device && !has_direct_fan && !has_hwmon_fan) ?
                              ("Cooling: " + std::to_string(g_fan_data.speed)).c_str() :
                              ("Speed: " + std::to_string(g_fan_data.speed) + " RPM").c_str();

    ImGui::PlotLines(graph_title,
        g_fan_data.history.data(),
        g_fan_data.history.size(),
        0,
        overlay_text,
        0.0f,
        g_fan_data.scale,
        ImVec2(0, 80));
}

void renderThermalTab() {
    // FPS Slider
    ImGui::SliderFloat("FPS##thermal", &g_thermal_data.fps, 1.0f, 60.0f);
    
    // Scale Slider
    ImGui::SliderFloat("Scale##thermal", &g_thermal_data.scale, 0.0f, 100.0f);
    
    // Animation Toggle
    ImGui::Checkbox("Animate##thermal", &g_thermal_data.animate);
    
    // Temperature Graph
    ImGui::PlotLines("Temperature", 
        g_thermal_data.history.data(), 
        g_thermal_data.history.size(),
        0, 
        ("Temp: " + std::to_string(g_thermal_data.current_temp) + "°C").c_str(),
        0.0f, 
        g_thermal_data.scale,
        ImVec2(0, 80));
}

// Main system window render function
void systemWindow(const char* id, ImVec2 size, ImVec2 position) {
    // Update data
    static auto last_update = std::chrono::steady_clock::now();
    auto now = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now - last_update);
    
    if (duration.count() >= 1000) { // Update every second
        updateSystemInfo();
        updateProcessStats();
        updateCPUData();
        updateFanData();
        updateThermalData();
        last_update = now;
    }

    // Begin window
    ImGui::Begin(id);
    ImGui::SetWindowSize(id, size);
    ImGui::SetWindowPos(id, position);

    // Render system information
    renderSystemInfo();
    
    ImGui::Separator();

    // Render tabs
    if (ImGui::BeginTabBar("SystemTabs")) {
        if (ImGui::BeginTabItem("CPU")) {
            renderCPUTab();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Fan")) {
            renderFanTab();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Thermal")) {
            renderThermalTab();
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }

    ImGui::End();
}
