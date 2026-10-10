// Ensure CMAKE Tools extension in VSCode is installed
// Open folder of files
// Ctrl+Shift+P -> CMake: Reconfigure, Build, Run
// Click on launch button below (sa blue na bar)

#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui.h"
#include "imgui_internal.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>
#include <algorithm>
#include <cmath>
#include <ctime>
#include <string>
#include <vector>
#include <cstring>

// global state
struct Shell {
    bool shuttingDown = false;
    double startShutdown = 0.0;
    bool allowedQuit = false; // only affected by PWR
    bool closeWarning = false;

    bool showTaskManager = false;

    // add other state vars dito for other features
};

struct Proc {
    std::string name;
    int pid;
    float baseCpu;
    float memMB;
    bool alive = true;
};

// Mutable process list so we can terminate processes
static std::vector<Proc> g_procs = {
    {"System",              4,    1.2f,  18.5f, true},
    {"csopesy_shell.exe",   1204, 3.4f,  64.2f, true},
    {"explorer.exe",        2210, 2.1f, 112.8f, true},
    {"TaskManager.exe",     3321, 0.8f,  24.6f, true},
    {"Notepad.exe",         4120, 0.1f,   9.3f, true},
    {"audio_service",       888,  0.4f,  15.0f, true},
    {"network_daemon",      912,  0.6f,  21.7f, true},
    {"scheduler.exe",       1500, 1.7f,  32.4f, true},
};

static int g_selectedProcIdx = -1;

static Shell shell;
static const float taskbarHeight = 52.0f; //

// added to display warning if hindi PWR ginamit
static void OnWindowClose(GLFWwindow* w) {
    if (!shell.allowedQuit) glfwSetWindowShouldClose(w, GLFW_FALSE);
    shell.closeWarning = true;  
}

static std::string ClockString() {
    std::time_t t = std::time(nullptr);
    std::tm lt{};
#ifdef _WIN32
    localtime_s(&lt, &t);
#else
    localtime_r(&t, &lt);
#endif
    char buf[96];
    std::strftime(buf, sizeof buf, "%A, %b %d, %Y | %I:%M:%S %p", &lt);
    return buf;
}

// Desktop wallpaper + clock
static void DrawDesktop() {
    ImGuiViewport* vp = ImGui::GetMainViewport();
    ImDrawList* dl = ImGui::GetBackgroundDrawList();   // always behind every window
    ImVec2 p0 = vp->Pos, p1 = vp->Pos + vp->Size;

    // gradient color; can change
    dl->AddRectFilledMultiColor(p0, p1, IM_COL32(70, 140, 235, 255), IM_COL32(70, 140, 235, 255),
                                IM_COL32(150, 205, 255, 255), IM_COL32(150, 205, 255, 255));

    // clock real time
    std::string clk = ClockString();
    ImVec2 sz = ImGui::CalcTextSize(clk.c_str());
    ImVec2 a(p1.x - sz.x - 28, p0.y + 8), b(p1.x - 8, p0.y + 8 + sz.y + 12);
    dl->AddRectFilled(a, b, IM_COL32(15, 15, 20, 220), 4.0f);
    dl->AddText(ImVec2(a.x + 10, a.y + 6), IM_COL32(240, 240, 240, 255), clk.c_str());
}

// Taskbar with PWR
static void DrawTaskbar() {
    ImGuiViewport* vp = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(ImVec2(vp->Pos.x, vp->Pos.y + vp->Size.y - taskbarHeight));
    ImGui::SetNextWindowSize(ImVec2(vp->Size.x, taskbarHeight));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10, 8));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.06f, 0.06f, 0.09f, 0.97f));
    ImGui::Begin("##taskbar", nullptr,
                 ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
                 ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoScrollbar);
    ImGui::PopStyleColor();
    ImGui::PopStyleVar(2);
    ImGui::BringWindowToDisplayFront(ImGui::GetCurrentWindow());

    // add other taskbar features here (start menu, app icons, etc.)
    if (ImGui::Button("Task Mgr", ImVec2(80, 36))) {
        shell.showTaskManager = !shell.showTaskManager;
    }

    // PWR button (right side)
    ImGui::SameLine(vp->Size.x - 68);
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.3f, 0.3f, 1));
    if (ImGui::Button("PWR", ImVec2(56, 36))) ImGui::OpenPopup("Shut down CSOPESY emulator?");
    ImGui::PopStyleColor();

    ImGui::SetNextWindowPos(vp->Pos + vp->Size * 0.5f, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    if (ImGui::BeginPopupModal("Shut down CSOPESY emulator?", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("All applications will be closed.");
        ImGui::Spacing();
        if (ImGui::Button("Shut Down", ImVec2(120, 0))) {
            shell.shuttingDown = true;
            shell.startShutdown = glfwGetTime();
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0))) ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
    }
    ImGui::End();
}

// Closing Window with X Warning
static void DrawCloseWarning() {
    if (!shell.closeWarning) return;
    ImGuiViewport* vp = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(vp->Pos + vp->Size * 0.5f, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    if (ImGui::Begin("System Warning", &shell.closeWarning,
                     ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse)) {
        ImGui::BringWindowToDisplayFront(ImGui::GetCurrentWindow());
        ImGui::Text("Please use the PWR button to shut down.");
        ImGui::Spacing();
        if (ImGui::Button("OK", ImVec2(100, 0))) shell.closeWarning = false;
    }
    ImGui::End();
}

static void DrawTaskManager() {
    if (!shell.showTaskManager) return;

    ImGui::SetNextWindowPos(ImVec2(100, 60), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(620, 480), ImGuiCond_FirstUseEver);

    if (ImGui::Begin("Task Manager", &shell.showTaskManager)) {
        if (ImGui::BeginTabBar("tm_tabs")) {

            
            // tab 1: Processes
            if (ImGui::BeginTabItem("Processes")) {
                float totalCpu = 0.0f;
                float totalMem = 0.0f;
                int activeCount = 0;
                float t = (float)glfwGetTime();

                // compute instantaneous animated CPU & tally active memory
                std::vector<float> currentCpu(g_procs.size());
                for (size_t i = 0; i < g_procs.size(); i++) {
                    if (!g_procs[i].alive) continue;
                    activeCount++;
                    // Sine-wave wobble so CPU usage feels alive
                    currentCpu[i] = std::max(0.0f, g_procs[i].baseCpu + 1.4f * std::sin(t * 1.5f + (float)i));
                    totalCpu += currentCpu[i];
                    totalMem += g_procs[i].memMB;
                }

                // reserve 36px at the bottom for footer metrics and buttons
                ImGuiTableFlags tableFlags = ImGuiTableFlags_Borders 
                                           | ImGuiTableFlags_RowBg 
                                           | ImGuiTableFlags_ScrollY 
                                           | ImGuiTableFlags_Resizable
                                           | ImGuiTableFlags_Sortable;

                if (ImGui::BeginTable("procs_table", 4, tableFlags, ImVec2(0, -38.0f))) {
                    ImGui::TableSetupScrollFreeze(0, 1);
                    ImGui::TableSetupColumn("Name",   ImGuiTableColumnFlags_WidthStretch | ImGuiTableColumnFlags_DefaultSort, 0.0f, 0);
                    ImGui::TableSetupColumn("PID",    ImGuiTableColumnFlags_WidthFixed, 65.0f, 1);
                    ImGui::TableSetupColumn("CPU",    ImGuiTableColumnFlags_WidthFixed, 75.0f, 2);
                    ImGui::TableSetupColumn("Memory", ImGuiTableColumnFlags_WidthFixed, 95.0f, 3);
                    ImGui::TableHeadersRow();

                    // table column sorting
                    if (ImGuiTableSortSpecs* sorts = ImGui::TableGetSortSpecs()) {
                        if (sorts->SpecsDirty && sorts->SpecsCount > 0) {
                            const ImGuiTableColumnSortSpecs* s = &sorts->Specs[0];
                            std::sort(g_procs.begin(), g_procs.end(), [&](const Proc& a, const Proc& b) {
                                bool asc = (s->SortDirection == ImGuiSortDirection_Ascending);
                                if (s->ColumnUserID == 0) return asc ? (a.name < b.name) : (a.name > b.name);
                                if (s->ColumnUserID == 1) return asc ? (a.pid < b.pid) : (a.pid > b.pid);
                                if (s->ColumnUserID == 2) return asc ? (a.baseCpu < b.baseCpu) : (a.baseCpu > b.baseCpu);
                                if (s->ColumnUserID == 3) return asc ? (a.memMB < b.memMB) : (a.memMB > b.memMB);
                                return false;
                            });
                            sorts->SpecsDirty = false;
                        }
                    }

                    for (size_t i = 0; i < g_procs.size(); i++) {
                        if (!g_procs[i].alive) continue;

                        ImGui::TableNextRow();
                        bool isSelected = (g_selectedProcIdx == (int)i);

                        // column 0: Name (Selectable spans the full row)
                        ImGui::TableNextColumn();
                        char label[128];
                        snprintf(label, sizeof(label), "%s##proc_%d", g_procs[i].name.c_str(), g_procs[i].pid);
                        if (ImGui::Selectable(label, isSelected, ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowItemOverlap)) {
                            g_selectedProcIdx = (int)i;
                        }

                        // column 1: PID
                        ImGui::TableNextColumn();
                        ImGui::Text("%d", g_procs[i].pid);

                        // column 2: CPU
                        ImGui::TableNextColumn();
                        ImGui::Text("%.1f%%", currentCpu[i]);

                        // column 3: Memory
                        ImGui::TableNextColumn();
                        ImGui::Text("%.1f MB", g_procs[i].memMB);
                    }
                    ImGui::EndTable();
                }

                // footer live status metrics
                ImGui::AlignTextToFramePadding();
                ImGui::Text("Processes: %d | CPU: %.1f%% | Memory: %.1f MB", activeCount, totalCpu, totalMem);

                // action buttons at the bottom - reset all and end task
                ImGui::SameLine(ImGui::GetContentRegionAvail().x - 170.0f);
                if (ImGui::Button("Reset All", ImVec2(75, 0))) {
                    for (auto& p : g_procs) p.alive = true;
                    g_selectedProcIdx = -1;
                }

                ImGui::SameLine();
                bool canEnd = (g_selectedProcIdx >= 0 && g_selectedProcIdx < (int)g_procs.size() && g_procs[g_selectedProcIdx].alive);
                if (!canEnd) ImGui::BeginDisabled();
                if (ImGui::Button("End Task", ImVec2(85, 0))) {
                    if (canEnd) {
                        g_procs[g_selectedProcIdx].alive = false;
                        g_selectedProcIdx = -1;
                    }
                }
                if (!canEnd) ImGui::EndDisabled();

                ImGui::EndTabItem();
            }

            // tab 2: performance (graph and progress bar)
            if (ImGui::BeginTabItem("Performance")) {
                static float hist[90] = {};
                static double lastT = 0.0;
                double now = glfwGetTime();

                // update scrolling CPU history buffer every 0.25 seconds
                if (now - lastT > 0.25) {
                    lastT = now;
                    std::memmove(hist, hist + 1, sizeof(float) * 89);
                    hist[89] = 22.0f + 14.0f * std::sin((float)now * 0.8f) + (std::rand() % 120) / 10.0f;
                }

                ImGui::Text("CPU Utilization");
                ImGui::PlotLines("##cpu_graph", hist, 90, 0, nullptr, 0.0f, 100.0f, ImVec2(-1, 140));

                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();

                ImGui::Text("Physical Memory (RAM)");
                // display memory relative to a 512 MB base
                float memFraction = 184.2f / 512.0f;
                ImGui::ProgressBar(memFraction, ImVec2(-1, 24.0f), "184.2 MB / 512.0 MB (36%)");

                ImGui::EndTabItem();
            }

            ImGui::EndTabBar();
        }
    }
    ImGui::End();
}

static void DrawApps() {
    // add other festures?
    DrawTaskManager();
}

// Shutdown transition
static void DrawShutdown(GLFWwindow* window) {
    const double dur = 2.0;
    double e = glfwGetTime() - shell.startShutdown;
    float a = (float)std::min(1.0, e / (dur * 0.6));
    ImGuiViewport* vp = ImGui::GetMainViewport();
    ImDrawList* dl = ImGui::GetForegroundDrawList();
    dl->AddRectFilled(vp->Pos, vp->Pos + vp->Size, IM_COL32(0, 0, 0, (int)(255 * a)));
    if (a > 0.9f) {
        const char* s = "Shutting down...";
        ImVec2 sz = ImGui::CalcTextSize(s);
        dl->AddText(vp->Pos + vp->Size * 0.5f - sz * 0.5f, IM_COL32(235, 235, 235, 255), s);
    }
    if (e >= dur) { shell.allowedQuit = true; glfwSetWindowShouldClose(window, GLFW_TRUE); }
}


int main() {
    if (!glfwInit()) return 1;
#if defined(__APPLE__)
    const char* glsl = "#version 150";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#else
    const char* glsl = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
#endif
    GLFWwindow* window = glfwCreateWindow(1280, 720, "CSOPESY Desktop OS Emulator", nullptr, nullptr);
    if (!window) { glfwTerminate(); return 1; }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    glfwSetWindowCloseCallback(window, OnWindowClose);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::GetIO().IniFilename = nullptr;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl);

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        DrawDesktop(); // desktop layer
        DrawApps(); // applications
        DrawTaskbar(); // taskbar (+ PWR)
        if (shell.shuttingDown) DrawShutdown(window);  // overlay
        if (shell.closeWarning) DrawCloseWarning(); // close warning

        ImGui::Render();
        int fw, fh;
        glfwGetFramebufferSize(window, &fw, &fh);
        glViewport(0, 0, fw, fh);
        glClearColor(0, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
