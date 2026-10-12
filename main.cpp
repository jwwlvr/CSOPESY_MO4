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
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <string>

// global state
struct Shell {
    bool shuttingDown = false;
    double startShutdown = 0.0;
    bool allowedQuit = false; // only affected by PWR
    bool closeWarning = false;

    // add other state vars dito for other features
    bool showNotes = false;
    bool showCalc = false;
    bool showTaskMgr = false;
};

static Shell shell;
static const float taskbarHeight = 52.0f; //
static const float appScale = 1.4f;  // size of the app windows

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
    // blue button = app is running
    bool notesOn = shell.showNotes;
    if (notesOn) ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.45f, 0.85f, 1.0f));
    if (ImGui::Button("NOTES", ImVec2(84, 36))) shell.showNotes = !shell.showNotes;
    if (notesOn) ImGui::PopStyleColor();

    ImGui::SameLine();
    bool calcOn = shell.showCalc;
    if (calcOn) ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.45f, 0.85f, 1.0f));
    if (ImGui::Button("CALC", ImVec2(84, 36))) shell.showCalc = !shell.showCalc;
    if (calcOn) ImGui::PopStyleColor();

    ImGui::SameLine();
    bool tasksOn = shell.showTaskMgr;
    if (tasksOn) ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.45f, 0.85f, 1.0f));
    if (ImGui::Button("TASKS", ImVec2(84, 36))) shell.showTaskMgr = !shell.showTaskMgr;
    if (tasksOn) ImGui::PopStyleColor();

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

// type notes sa memory - close and stay lang sya dun
static void DrawNotes() {
    if (!shell.showNotes) return;
    static char text[1024] = "Type your notes here...";
    ImGui::SetNextWindowSize(ImVec2(400 * appScale, 280 * appScale), ImGuiCond_FirstUseEver);
    ImGui::Begin("Notes", &shell.showNotes);
    ImGui::SetWindowFontScale(appScale);
    ImGui::InputTextMultiline("##notes", text, sizeof text, ImVec2(-1, -1));
    ImGui::End();
}

static std::string Calc(double a, double b, char op) {
    double r = 0;
    if (op == '+') r = a + b;
    else if (op == '-') r = a - b;
    else if (op == '*') r = a * b;
    else if (op == '/') {
        if (b == 0) return "Error";
        r = a / b;
    }
    char buf[64];
    snprintf(buf, sizeof buf, "%.10g", r);
    return buf;
}

// simple calculator
static void DrawCalculator() {
    if (!shell.showCalc) return;
    static std::string display = "0";
    static double acc = 0;    
    static char op = 0;        
    static bool newNum = true;
    const char* keys[] = { "7", "8", "9", "/",
                           "4", "5", "6", "*",
                           "1", "2", "3", "-",
                           "0", ".", "C", "+" };

    ImGui::Begin("Calculator", &shell.showCalc, ImGuiWindowFlags_AlwaysAutoResize);
    ImGui::SetWindowFontScale(2.0f * appScale);
    ImGui::Text("%s", display.c_str());
    ImGui::SetWindowFontScale(appScale);
    ImGui::Separator();

    for (int i = 0; i < 16; i++) {
        if (i % 4 != 0) ImGui::SameLine();
        if (ImGui::Button(keys[i], ImVec2(56 * appScale, 45 * appScale))) {
            char k = keys[i][0];
            if ((k >= '0' && k <= '9') || k == '.') {
                if (newNum) {
                    display = (k == '.') ? "0." : std::string(1, k);
                    newNum = false;
                } else if (display.size() < 12 && (k != '.' || display.find('.') == std::string::npos)) {
                    display += k;
                }
            } else if (k == 'C') {
                display = "0"; acc = 0; op = 0; newNum = true;
            } else {
                if (op != 0 && !newNum) display = Calc(acc, atof(display.c_str()), op);
                acc = atof(display.c_str());
                op = k;
                newNum = true;
            }
        }
    }
    if (ImGui::Button("=", ImVec2(224 * appScale + 24, 45 * appScale))) {
        if (op != 0) {
            display = Calc(acc, atof(display.c_str()), op);
            op = 0; acc = 0; newNum = true;
        }
    }
    ImGui::End();
}

// dummy values lang
struct Proc {
    const char* name;
    int pid;
    float cpu;
    float mem;
};

static void DrawTaskManager() {
    if (!shell.showTaskMgr) return;
    static Proc procs[] = {
        { "System", 4, 1.2f, 0.1f },
        { "explorer.exe", 1120, 2.5f, 120.4f },
        { "csopesy_shell.exe", 2048, 4.8f, 85.2f },
        { "chrome.exe", 3320, 12.3f, 640.7f },
        { "vscode.exe", 4012, 7.9f, 410.5f },
        { "spotify.exe", 876, 0.6f, 32.8f },
        { "camera.exe", 1544, 0.3f, 18.1f },
        { "zoom.exe", 918, 2.8f, 91.8f },
        { "notes.exe", 5120, 0.1f, 9.6f },
        { "settings.exe", 5188, 0.0f, 12.3f },
        { "calculator.exe", 1009, 1.2f, 54.2f },
        { "taskmgr.exe", 6004, 1.4f, 24.9f }
    };

    int n = sizeof(procs) / sizeof(procs[0]);
    ImGui::Begin("Task Manager", &shell.showTaskMgr, ImGuiWindowFlags_AlwaysAutoResize);
    ImGui::SetWindowFontScale(appScale);
    ImGui::Text("Processes (click a column to sort)");
    if (ImGui::BeginTable("procs", 4, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Sortable)) {
        ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthFixed, 200 * appScale);
        ImGui::TableSetupColumn("PID", ImGuiTableColumnFlags_WidthFixed, 60 * appScale);
        ImGui::TableSetupColumn("CPU", ImGuiTableColumnFlags_WidthFixed, 70 * appScale);
        ImGui::TableSetupColumn("Memory", ImGuiTableColumnFlags_WidthFixed, 90 * appScale);
        ImGui::TableHeadersRow();

        // sort when a header is clicked
        ImGuiTableSortSpecs* specs = ImGui::TableGetSortSpecs();
        if (specs && specs->SpecsDirty && specs->SpecsCount > 0) {
            int col = specs->Specs[0].ColumnIndex;
            bool asc = specs->Specs[0].SortDirection == ImGuiSortDirection_Ascending;
            std::sort(procs, procs + n, [col, asc](const Proc& a, const Proc& b) {
                bool less, greater;
                if (col == 0) { less = std::string(a.name) < b.name; greater = std::string(b.name) < a.name; }
                else if (col == 1) { less = a.pid < b.pid; greater = a.pid > b.pid; }
                else if (col == 2) { less = a.cpu < b.cpu; greater = a.cpu > b.cpu; }
                else { less = a.mem < b.mem; greater = a.mem > b.mem; }
                return asc ? less : greater;
            });
            specs->SpecsDirty = false;
        }

        for (int i = 0; i < n; i++) {
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0); ImGui::Text("%s", procs[i].name);
            ImGui::TableSetColumnIndex(1); ImGui::Text("%d", procs[i].pid);
            ImGui::TableSetColumnIndex(2); ImGui::Text("%.1f%%", procs[i].cpu);
            ImGui::TableSetColumnIndex(3); ImGui::Text("%.1f MB", procs[i].mem);
        }
        ImGui::EndTable();
    }
    ImGui::End();
}

static void DrawApps() {
    // add other features?
    DrawNotes();
    DrawCalculator();
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
