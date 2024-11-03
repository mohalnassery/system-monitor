#include "header.h"

// Network interface data structure
struct NetworkInterface {
    std::string name;        // e.g., "lo", "wlp5s0"
    std::string ipv4;        // IPv4 address
    
    // RX Statistics
    RX rx;
    // TX Statistics 
    TX tx;

    // History for graphs
    std::vector<float> rx_history;
    std::vector<float> tx_history;
};

class NetworkMonitor {
private:
    std::vector<NetworkInterface> interfaces;
    const float MAX_SCALE_GB = 2.0f; // 2GB scale for progress bars
    const size_t HISTORY_SIZE = 100;  // Size of history for graphs

    // Convert bytes to appropriate unit string (KB/MB/GB)
    std::string formatBytes(uint64_t bytes) {
        const double kb = bytes / 1024.0;
        const double mb = kb / 1024.0;
        const double gb = mb / 1024.0;
        
        char buffer[32];
        if (mb >= 100 && mb < 1024) {
            snprintf(buffer, sizeof(buffer), "%.2f MB", mb);
        } else if (gb >= 0.5) {
            snprintf(buffer, sizeof(buffer), "%.2f GB", gb);
        } else {
            snprintf(buffer, sizeof(buffer), "%.2f KB", kb);
        }
        return std::string(buffer);
    }

    void updateInterfaces() {
        struct ifaddrs *ifaddr, *ifa;
        if (getifaddrs(&ifaddr) == -1) {
            perror("getifaddrs");
            return;
        }

        // Clear old interfaces that might not exist anymore
        interfaces.clear();
        
        for (ifa = ifaddr; ifa != nullptr; ifa = ifa->ifa_next) {
            if (ifa->ifa_addr == nullptr) continue;

            if (ifa->ifa_addr->sa_family == AF_INET) {
                NetworkInterface iface;
                iface.name = ifa->ifa_name;
                
                char ip[INET_ADDRSTRLEN];
                inet_ntop(AF_INET, 
                    &((struct sockaddr_in *)ifa->ifa_addr)->sin_addr,
                    ip, INET_ADDRSTRLEN);
                iface.ipv4 = ip;
                
                interfaces.push_back(iface);
            }
        }
        
        freeifaddrs(ifaddr);
    }

    void updateStatistics() {
        std::ifstream netdev("/proc/net/dev");
        std::string line;
        
        // Skip header lines
        std::getline(netdev, line);
        std::getline(netdev, line);
        
        while (std::getline(netdev, line)) {
            std::istringstream iss(line);
            std::string name;
            iss >> name;
            
            // Remove colon from interface name
            name = name.substr(0, name.length() - 1);
            
            // Find the interface in our list
            auto it = std::find_if(interfaces.begin(), interfaces.end(),
                [&name](const NetworkInterface& iface) {
                    return iface.name == name;
                });
                
            if (it != interfaces.end()) {
                // Read RX stats
                iss >> it->rx.bytes >> it->rx.packets 
                    >> it->rx.errs >> it->rx.drop
                    >> it->rx.fifo >> it->rx.frame
                    >> it->rx.compressed >> it->rx.multicast;
                    
                // Read TX stats
                iss >> it->tx.bytes >> it->tx.packets
                    >> it->tx.errs >> it->tx.drop
                    >> it->tx.fifo >> it->tx.colls
                    >> it->tx.carrier >> it->tx.compressed;

                // Update history
                if (it->rx_history.size() >= HISTORY_SIZE) {
                    it->rx_history.erase(it->rx_history.begin());
                    it->tx_history.erase(it->tx_history.begin());
                }
                
                float rx_gb = it->rx.bytes / (1024.0f * 1024.0f * 1024.0f);
                float tx_gb = it->tx.bytes / (1024.0f * 1024.0f * 1024.0f);
                it->rx_history.push_back(rx_gb);
                it->tx_history.push_back(tx_gb);
            }
        }
    }

    void renderProgressBar(const std::string& label, float value_gb) {
        float fraction = value_gb / MAX_SCALE_GB;
        fraction = std::min(fraction, 1.0f); // Clamp to max scale
        
        ImGui::Text("%s", label.c_str());
        ImGui::ProgressBar(fraction, ImVec2(-1.0f, 0.0f));
    }

public:
    void update() {
        updateInterfaces();
        updateStatistics();
    }

    void renderInterfaces() {
        for (const auto& iface : interfaces) {
            if (ImGui::CollapsingHeader(iface.name.c_str())) {
                ImGui::Text("IPv4: %s", iface.ipv4.c_str());
            }
        }
    }

    void renderRXTable() {
        if (ImGui::BeginTable("RX_Stats", 8, 
            ImGuiTableFlags_Borders | 
            ImGuiTableFlags_Resizable | 
            ImGuiTableFlags_ScrollY)) {
            
            ImGui::TableSetupColumn("Bytes");
            ImGui::TableSetupColumn("Packets");
            ImGui::TableSetupColumn("Errors");
            ImGui::TableSetupColumn("Drops");
            ImGui::TableSetupColumn("FIFO");
            ImGui::TableSetupColumn("Frame");
            ImGui::TableSetupColumn("Compressed");
            ImGui::TableSetupColumn("Multicast");
            ImGui::TableHeadersRow();
            
            for (const auto& iface : interfaces) {
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::Text("%s", formatBytes(iface.rx.bytes).c_str());
                ImGui::TableNextColumn();
                ImGui::Text("%d", iface.rx.packets);
                ImGui::TableNextColumn();
                ImGui::Text("%d", iface.rx.errs);
                ImGui::TableNextColumn();
                ImGui::Text("%d", iface.rx.drop);
                ImGui::TableNextColumn();
                ImGui::Text("%d", iface.rx.fifo);
                ImGui::TableNextColumn();
                ImGui::Text("%d", iface.rx.frame);
                ImGui::TableNextColumn();
                ImGui::Text("%d", iface.rx.compressed);
                ImGui::TableNextColumn();
                ImGui::Text("%d", iface.rx.multicast);
            }
            
            ImGui::EndTable();
        }
    }

    void renderTXTable() {
        if (ImGui::BeginTable("TX_Stats", 8, 
            ImGuiTableFlags_Borders | 
            ImGuiTableFlags_Resizable | 
            ImGuiTableFlags_ScrollY)) {
            
            ImGui::TableSetupColumn("Bytes");
            ImGui::TableSetupColumn("Packets");
            ImGui::TableSetupColumn("Errors");
            ImGui::TableSetupColumn("Drops");
            ImGui::TableSetupColumn("FIFO");
            ImGui::TableSetupColumn("Collisions");
            ImGui::TableSetupColumn("Carrier");
            ImGui::TableSetupColumn("Compressed");
            ImGui::TableHeadersRow();
            
            for (const auto& iface : interfaces) {
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::Text("%s", formatBytes(iface.tx.bytes).c_str());
                ImGui::TableNextColumn();
                ImGui::Text("%d", iface.tx.packets);
                ImGui::TableNextColumn();
                ImGui::Text("%d", iface.tx.errs);
                ImGui::TableNextColumn();
                ImGui::Text("%d", iface.tx.drop);
                ImGui::TableNextColumn();
                ImGui::Text("%d", iface.tx.fifo);
                ImGui::TableNextColumn();
                ImGui::Text("%d", iface.tx.colls);
                ImGui::TableNextColumn();
                ImGui::Text("%d", iface.tx.carrier);
                ImGui::TableNextColumn();
                ImGui::Text("%d", iface.tx.compressed);
            }
            
            ImGui::EndTable();
        }
    }

    void renderRXVisualizations() {
        for (const auto& iface : interfaces) {
            float rx_gb = iface.rx.bytes / (1024.0f * 1024.0f * 1024.0f);
            std::string label = iface.name + " RX: " + formatBytes(iface.rx.bytes);
            renderProgressBar(label, rx_gb);
        }
    }

    void renderTXVisualizations() {
        for (const auto& iface : interfaces) {
            float tx_gb = iface.tx.bytes / (1024.0f * 1024.0f * 1024.0f);
            std::string label = iface.name + " TX: " + formatBytes(iface.tx.bytes);
            renderProgressBar(label, tx_gb);
        }
    }
};

// Global network monitor instance
static NetworkMonitor networkMonitor;

void networkWindow(const char* id, ImVec2 size, ImVec2 position) {
    ImGui::Begin(id);
    ImGui::SetWindowSize(id, size);
    ImGui::SetWindowPos(id, position);

    // Update network statistics
    networkMonitor.update();

    // Network interfaces section
    if (ImGui::BeginTabBar("NetworkTabs")) {
        if (ImGui::BeginTabItem("Interfaces")) {
            networkMonitor.renderInterfaces();
            ImGui::EndTabItem();
        }
        
        if (ImGui::BeginTabItem("RX")) {
            networkMonitor.renderRXTable();
            ImGui::Spacing();
            networkMonitor.renderRXVisualizations();
            ImGui::EndTabItem();
        }
        
        if (ImGui::BeginTabItem("TX")) {
            networkMonitor.renderTXTable();
            ImGui::Spacing();
            networkMonitor.renderTXVisualizations();
            ImGui::EndTabItem();
        }
        
        ImGui::EndTabBar();
    }

    ImGui::End();
}
