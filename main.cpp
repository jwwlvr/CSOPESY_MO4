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

// global state
struct Shell {
    bool shuttingDown = false;
    double startShutdown = 0.0;
    bool allowedQuit = false; // only affected by PWR
    bool closeWarning = false;

    // add other state vars dito for other features
};

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

static void DrawApps() {
    // add other festures?
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
