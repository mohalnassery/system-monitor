#ifndef header_H
#define header_H

#include <GL/glew.h>
#include <SDL.h>
#include <SDL_opengl.h>

#include "imgui.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_opengl3.h"

#include <stdio.h>
#include <dirent.h>
#include <vector>
#include <iostream>
#include <cmath>
#include <fstream>
#include <unistd.h>
#include <limits.h>
#include <cpuid.h>
#include <sys/types.h>
#include <sys/sysinfo.h>
#include <sys/statvfs.h>
#include <ctime>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <map>
#include <chrono>
#include <sstream>
#include <algorithm>

using namespace std;

struct CPUStats
{
    long long int user;
    long long int nice;
    long long int system;
    long long int idle;
    long long int iowait;
    long long int irq;
    long long int softirq;
    long long int steal;
    long long int guest;
    long long int guestNice;
};

struct Proc
{
    int pid;
    string name;
    char state;
    long long int vsize;
    long long int rss;
    long long int utime;
    long long int stime;
};

struct IP4
{
    char *name;
    char addressBuffer[INET_ADDRSTRLEN];
};

struct Networks
{
    vector<IP4> ip4s;
};

struct TX
{
    int bytes;
    int packets;
    int errs;
    int drop;
    int fifo;
    int colls;
    int carrier;
    int compressed;
    int multicast;
};

struct RX
{
    int bytes;
    int packets;
    int errs;
    int drop;
    int fifo;
    int frame;
    int compressed;
    int multicast;
};

struct ProcessStats {
    long long int utime;
    long long int stime;
    long long int cutime;
    long long int cstime;
    long long int starttime;
    long long int vsize;
    long long int rss;
};

struct ProcessMetrics {
    float cpu_usage;
    float mem_usage;
    static ProcessStats getProcessStats(pid_t pid);
    static ProcessMetrics getProcessMetrics(
        const ProcessStats& current,
        const ProcessStats& last,
        unsigned long uptime
    );
};

struct ProcessInfo {
    pid_t pid;
    std::string name;
    std::string state;
    float cpu_usage;
    float mem_usage;
    bool selected;
    ProcessStats last_stats;
};

class ProcessManager {
private:
    std::vector<ProcessInfo> processes;
    std::string filter;
    std::vector<pid_t> selected_pids;
    std::map<pid_t, ProcessStats> last_stats;

public:
    void update();
    void render();
    bool matchesFilter(const ProcessInfo& proc);
    void handleSelection(ProcessInfo& proc);
};

void systemWindow(const char* id, ImVec2 size, ImVec2 position);
string readFileContent(const string& path);
float calculateCPUUsage();
string CPUinfo();
const char *getOsName();

// Network monitor window function declaration
void networkWindow(const char* id, ImVec2 size, ImVec2 position);

// Add this with the other window declarations
void memoryProcessesWindow(const char* id, ImVec2 size, ImVec2 position);

class ApplicationContext {
private:
    SDL_Window* window;
    SDL_GLContext gl_context;
    ImGuiIO* io;
    
public:
    ApplicationContext() : window(nullptr), gl_context(nullptr), io(nullptr) {}
    ~ApplicationContext();
    SDL_Window* getWindow() { return window; }
    void initialize();
};

class WindowManager {
private:
    struct WindowState {
        bool visible;
        ImVec2 position;
        ImVec2 size;
    };
    
    std::map<std::string, WindowState> window_states;
    
public:
    void initialize();
    void render();
};

class PerformanceManager {
private:
    const float TARGET_FRAMERATE = 60.0f;
    const float FRAME_TIME = 1000.0f / TARGET_FRAMERATE;
    std::chrono::steady_clock::time_point last_frame;
    
public:
    void limitFrameRate();
};

class Application {
private:
    ApplicationContext context;
    WindowManager window_manager;
    PerformanceManager perf_manager;
    bool running;
    
public:
    Application() : running(true) {}
    void run();
};

#endif
