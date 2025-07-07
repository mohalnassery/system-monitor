#include <GL/glew.h>
#include <SDL.h>
#include <SDL_opengl.h>
#include <fstream>
#include <dirent.h>
#include <ctype.h>
#include <cstring>
#include <cstdlib>

#include "header.h"

/*
NOTE : You are free to change the code as you wish, the main objective is to make the
       application work and pass the audit.

       It will be provided the main function with the following functions :

       - `void systemWindow(const char *id, ImVec2 size, ImVec2 position)`
            This function will draw the system window on your screen
       - `void memoryProcessesWindow(const char *id, ImVec2 size, ImVec2 position)`
            This function will draw the memory and processes window on your screen
       - `void networkWindow(const char *id, ImVec2 size, ImVec2 position)`
            This function will draw the network window on your screen
*/

// Main code
int main(int, char **)
{
    // Setup SDL
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) != 0)
    {
        printf("Error: %s\n", SDL_GetError());
        return -1;
    }

    // GL 3.0 + GLSL 130
    const char *glsl_version = "#version 130";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);

    // Create window with graphics context
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    SDL_Window *window = SDL_CreateWindow("System Monitor", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, window_flags);
    SDL_GLContext gl_context = SDL_GL_CreateContext(window);
    SDL_GL_MakeCurrent(window, gl_context);
    SDL_GL_SetSwapInterval(1); // Enable vsync

    // Initialize GLEW
    GLenum err = glewInit();
    if (err != GLEW_OK) {
        fprintf(stderr, "Failed to initialize GLEW: %s\n", glewGetErrorString(err));
        return 1;
    }

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    // render bindings
    ImGuiIO &io = ImGui::GetIO();

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer backends
    ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // background color
    // note : you are free to change the style of the application
    ImVec4 clear_color = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);

    // Main loop
    bool done = false;
    while (!done)
    {
        // Poll and handle events (inputs, window resize, etc.)
        // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
        // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application.
        // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application.
        // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            ImGui_ImplSDL2_ProcessEvent(&event);
            if (event.type == SDL_QUIT)
                done = true;
            if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(window))
                done = true;
        }

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame(window);
        ImGui::NewFrame();

        {
            ImVec2 mainDisplay = io.DisplaySize;
            memoryProcessesWindow("== Memory and Processes ==",
                                  ImVec2((mainDisplay.x / 2) - 20, (mainDisplay.y / 2) + 30),
                                  ImVec2((mainDisplay.x / 2) + 10, 10));
            // --------------------------------------
            systemWindow("== System ==",
                         ImVec2((mainDisplay.x / 2) - 10, (mainDisplay.y / 2) + 30),
                         ImVec2(10, 10));
            // --------------------------------------
            networkWindow("== Network ==",
                          ImVec2(mainDisplay.x - 20, (mainDisplay.y / 2) - 60),
                          ImVec2(10, (mainDisplay.y / 2) + 50));
        }

        // Rendering
        ImGui::Render();
        glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
        glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        SDL_GL_SwapWindow(window);
    }

    // Print debug statistics before cleanup
    printf("\n=== SYSTEM MONITOR DEBUG STATISTICS ===\n");

    // Memory statistics
    std::ifstream meminfo("/proc/meminfo");
    std::string line;
    size_t mem_total = 0, mem_available = 0, swap_total = 0, swap_free = 0;

    while (std::getline(meminfo, line)) {
        if (line.find("MemTotal:") == 0)
            sscanf(line.c_str(), "MemTotal: %zu kB", &mem_total);
        else if (line.find("MemAvailable:") == 0)
            sscanf(line.c_str(), "MemAvailable: %zu kB", &mem_available);
        else if (line.find("SwapTotal:") == 0)
            sscanf(line.c_str(), "SwapTotal: %zu kB", &swap_total);
        else if (line.find("SwapFree:") == 0)
            sscanf(line.c_str(), "SwapFree: %zu kB", &swap_free);
    }

    printf("Debug: mem_total=%zu, mem_available=%zu\n", mem_total, mem_available);
    if (mem_total > 0 && mem_available > 0) {
        size_t mem_used = mem_total - mem_available;
        float mem_percent = ((float)mem_used / mem_total) * 100.0f;
        printf("RAM: %.1f%% (%zu MB used / %zu MB total)\n",
               mem_percent, mem_used/1024, mem_total/1024);
    } else {
        printf("RAM: Could not read memory information\n");
    }

    printf("Debug: swap_total=%zu, swap_free=%zu\n", swap_total, swap_free);
    if (swap_total > 0) {
        size_t swap_used = swap_total - swap_free;
        float swap_percent = ((float)swap_used / swap_total) * 100.0f;
        printf("SWAP: %.1f%% (%zu MB used / %zu MB total)\n",
               swap_percent, swap_used/1024, swap_total/1024);
    } else {
        printf("SWAP: No swap space detected (total=%zu)\n", swap_total);
    }

    // Process count
    int process_count = 0;
    DIR* proc_dir = opendir("/proc");
    if (proc_dir) {
        struct dirent* entry;
        while ((entry = readdir(proc_dir)) != nullptr) {
            if (isdigit(entry->d_name[0])) {
                process_count++;
            }
        }
        closedir(proc_dir);
    }
    printf("Processes: %d total detected\n", process_count);

    // Sample process memory calculation
    printf("Sample process memory calculation:\n");
    long page_size = sysconf(_SC_PAGE_SIZE);
    printf("  Page size: %ld bytes\n", page_size);

    FILE* self_stat = fopen("/proc/self/stat", "r");
    if (self_stat) {
        // Read the entire line and parse field 24 (RSS)
        char line[1024];
        if (fgets(line, sizeof(line), self_stat)) {
            char* token = strtok(line, " ");
            long rss = 0;
            for (int i = 1; i < 24 && token; i++) {
                token = strtok(NULL, " ");
                if (i == 23 && token) { // Field 24 (0-based index 23)
                    rss = atol(token);
                }
            }

            if (rss > 0) {
                unsigned long process_memory = rss * page_size;
                float mem_percentage = ((double)process_memory / (double)(mem_total * 1024)) * 100.0;
                printf("  Monitor process: %ld pages = %lu bytes = %.3f%% of RAM\n",
                       rss, process_memory, mem_percentage);
            } else {
                printf("  Could not read RSS from /proc/self/stat\n");
            }
        }
        fclose(self_stat);
    }

    printf("=======================================\n\n");

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    SDL_GL_DeleteContext(gl_context);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
