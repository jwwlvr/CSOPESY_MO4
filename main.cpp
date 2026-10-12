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
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <string>
#include <vector>

//function declaration
static void OnWindowClose(GLFWwindow* w);
static std::string ClockString();

// ui elements
static void DrawBios();
static void DrawSplash();
static void DrawDesktop();
static void DrawTaskbar();
static void DrawCloseWarning();
static void DrawTaskManager();
static void DrawNotepad();
static void DrawApps();
static void DrawShutdown(GLFWwindow* window);

// buttons
static bool IconButton(const char* id, int kind, bool active, const char* tooltip);
static bool PowerButton(const char* id, const char* tooltip);

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
    bool booted = false;   // false = BIOS screen still showing
    bool splashDone = false;   // false = splash screen still showing
    double splashStart = 0.0;
};

// dummy values lang
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
static const float appScale = 1.4f;  // size of the app windows
static const double biosTime = 17.5;    // seconds
static const double splashTime = 5.0;   // seconds

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

// BIOS screen, then goes to the desktop
static void DrawBios() {
    double t = glfwGetTime();   // seconds since the program started
    ImGuiViewport* vp = ImGui::GetMainViewport();
    ImDrawList* dl = ImGui::GetBackgroundDrawList();
    dl->AddRectFilled(vp->Pos, vp->Pos + vp->Size, IM_COL32(0, 0, 0, 255));

    const char* lines[] = {
        "CSOPESY Megatrends",
        "Released: 12/10/2026",
        "DLSU CCS (C) 2026 CSOPESY Megatrends Inc.",
        "",
        "Memory Test:",
        "Checking RAM : 64000K OK",
        "",
        "CPU Type: Pentium III",
        "BIOS Version: BCN SIT 1989-1994 Special CSOPESY",
        "Main Processor: Pentium III",
        "Numeric Processor: Built-in",
        "",
        "Primary Master:   420 MB WDCAC2420H",
        "Primary Slave:    None",
        "Secondary Master: CD-ROM LTN-385A",
        "Secondary Slave:  None"
    };
    int total = sizeof(lines) / sizeof(lines[0]);
    int count = (int)(t / 0.7) + 1;   // show one more line every 0.7s
    if (count > total) count = total;
    for (int i = 0; i < count; i++)
        dl->AddText(ImVec2(vp->Pos.x + 14, vp->Pos.y + 12 + i * 18), IM_COL32(220, 220, 220, 255), lines[i]);

    dl->AddText(ImVec2(vp->Pos.x + vp->Size.x * 0.5f - 30, vp->Pos.y + vp->Size.y * 0.35f),
                IM_COL32(120, 120, 120, 255), "Loading...");

    if (t >= biosTime) {
        shell.booted = true;
        shell.splashStart = t;
    }
}

// Splash screen after BIOS, then goes to the desktop
static void DrawSplash() {
    double t = glfwGetTime() - shell.splashStart;
    ImGuiViewport* vp = ImGui::GetMainViewport();
    ImDrawList* dl = ImGui::GetBackgroundDrawList();
    dl->AddRectFilled(vp->Pos, vp->Pos + vp->Size, IM_COL32(0, 0, 0, 255));

    const char* logo[] = {
        R"ASCII(____  ____   ___  ____  _____ ____  __   __)ASCII",
        R"ASCII( / ___|/ ___|  / _ \ |  _ \ | ____|/ ___| \ \ / /)ASCII",
        R"ASCII(| |    \___ \ | | | || |_) ||  _|  \___ \  \ V / )ASCII",
        R"ASCII(| |___  ___) || |_| ||  __/ | |___  ___) |  | |  )ASCII",
        R"ASCII( \____||____/  \___/ |_|    |_____||____/   |_|  )ASCII"
    };
    float scale = 1.6f;
    float fs = ImGui::GetFontSize() * scale;
    float y = vp->Pos.y + vp->Size.y * 0.25f;
    for (int i = 0; i < 5; i++) {
        float w = ImGui::CalcTextSize(logo[i]).x * scale;
        dl->AddText(ImGui::GetFont(), fs, ImVec2(vp->Pos.x + (vp->Size.x - w) * 0.5f, y + i * fs),
                    IM_COL32(60, 200, 220, 255), logo[i]);
    }

    const char* info[] = {
        "CSOPESY Operating System Emulator v1.0",
        "Created By: Austria, De Leon, Guerrero, and Patricio",
        "",
        "Major Output 4 (MO4)"
    };
    y += 5 * fs + 30;
    for (int i = 0; i < 4; i++) {
        float w = ImGui::CalcTextSize(info[i]).x;
        dl->AddText(ImVec2(vp->Pos.x + (vp->Size.x - w) * 0.5f, y + i * 18),
                    IM_COL32(220, 220, 220, 255), info[i]);
    }

    // loading dots
    int dots = (int)(t * 2) % 3 + 1;
    std::string ld = "Loading" + std::string(dots, '.');
    float lw = ImGui::CalcTextSize("Loading...").x;
    dl->AddText(ImVec2(vp->Pos.x + (vp->Size.x - lw) * 0.5f, y + 6 * 18 + 40),
                IM_COL32(60, 220, 60, 255), ld.c_str());

    if (t >= splashTime) shell.splashDone = true;
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


    // bool notesOn = shell.showNotes;
    // if (notesOn) ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.45f, 0.85f, 1.0f));
    // if (ImGui::Button("NOTES", ImVec2(84, 36))) shell.showNotes = !shell.showNotes;
    // if (notesOn) ImGui::PopStyleColor();

    // ImGui::SameLine();
    // bool calcOn = shell.showCalc;
    // if (calcOn) ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.45f, 0.85f, 1.0f));
    // if (ImGui::Button("CALC", ImVec2(84, 36))) shell.showCalc = !shell.showCalc;
    // if (calcOn) ImGui::PopStyleColor();

    // ImGui::SameLine();
    // bool tasksOn = shell.showTaskMgr;
    // if (tasksOn) ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.45f, 0.85f, 1.0f));
    // if (ImGui::Button("TASKS", ImVec2(84, 36))) shell.showTaskMgr = !shell.showTaskMgr;
    // if (tasksOn) ImGui::PopStyleColor();

    // ImGui::SameLine(vp->Size.x - 68);
    // ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.3f, 0.3f, 1));
    // if (ImGui::Button("PWR", ImVec2(56, 36))) ImGui::OpenPopup("Shut down CSOPESY emulator?");
    // ImGui::PopStyleColor();

    //task manager button
    if (IconButton("##tm_icon", 0, shell.showTaskMgr, "Task Manager")) {
        shell.showTaskMgr = !shell.showTaskMgr;
    }

    //notepad button
    ImGui::SameLine();
    if (IconButton("##np_icon", 1, shell.showNotes, "Notepad")) {
        shell.showNotes = !shell.showNotes;
    }
    // calculator button
    ImGui::SameLine();
    if (IconButton("##calc_icon", 2, shell.showCalc, "Calculator")) {
        shell.showCalc = !shell.showCalc;
    }

    // PWR button (right side)
    ImGui::SameLine(vp->Size.x - 56);
    if (PowerButton("##pwr_btn", "Shut Down CSOPESY OS")) {
        ImGui::OpenPopup("Shut down CSOPESY emulator?");
    }

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

// app icon button 
static bool IconButton(const char* id, int kind, bool active, const char* tooltip) {
    const ImVec2 size(44, 36);
    ImVec2 pos = ImGui::GetCursorScreenPos();
    bool clicked = ImGui::InvisibleButton(id, size);
    bool hovered = ImGui::IsItemHovered();
    ImDrawList* dl = ImGui::GetWindowDrawList();

    // Background highlight
    ImU32 bg = active  ? IM_COL32(70, 110, 200, 255)
             : hovered ? IM_COL32(60, 60, 85, 255)
                       : IM_COL32(35, 35, 50, 255);
    ImU32 fg = IM_COL32(240, 240, 240, 255);
    dl->AddRectFilled(pos, pos + size, bg, 6.0f);

    ImVec2 c = pos + size * 0.5f;

    // task manager button
    if (kind == 0) {
        ImVec2 p1(c.x - 13, c.y);
        ImVec2 p2(c.x - 6,  c.y);
        ImVec2 p3(c.x - 2,  c.y - 9);  // peak
        ImVec2 p4(c.x + 3,  c.y + 8);  // dip
        ImVec2 p5(c.x + 7,  c.y);
        ImVec2 p6(c.x + 13, c.y);

        dl->AddLine(p1, p2, fg, 2.0f);
        dl->AddLine(p2, p3, fg, 2.0f);
        dl->AddLine(p3, p4, fg, 2.0f);
        dl->AddLine(p4, p5, fg, 2.0f);
        dl->AddLine(p5, p6, fg, 2.0f);
    }

    else if (kind == 1) {
        // paper sheet rectangle
        dl->AddRect(ImVec2(c.x - 9, c.y - 11), ImVec2(c.x + 9, c.y + 11), fg, 2.0f, 0, 1.8f);
        // 3 horizontal text lines
        for (int i = 0; i < 3; i++) {
            dl->AddLine(ImVec2(c.x - 5, c.y - 5 + i * 5), ImVec2(c.x + 5, c.y - 5 + i * 5), fg, 1.8f);
        }
    }

    else if (kind == 2) {
        // Outer calculator body
        dl->AddRect(ImVec2(c.x - 10, c.y - 12), ImVec2(c.x + 10, c.y + 12), fg, 2.5f, 0, 1.6f);

        // Top digital readout screen
        dl->AddRectFilled(ImVec2(c.x - 7, c.y - 9), ImVec2(c.x + 7, c.y - 4), fg);

        // 2x2 Grid of keypad buttons
        dl->AddRectFilled(ImVec2(c.x - 7, c.y - 1), ImVec2(c.x - 2, c.y + 3), fg, 1.0f); // Top-left
        dl->AddRectFilled(ImVec2(c.x + 2, c.y - 1), ImVec2(c.x + 7, c.y + 3), fg, 1.0f); // Top-right
        dl->AddRectFilled(ImVec2(c.x - 7, c.y + 5), ImVec2(c.x - 2, c.y + 9), fg, 1.0f); // Bottom-left
        dl->AddRectFilled(ImVec2(c.x + 2, c.y + 5), ImVec2(c.x + 7, c.y + 9), fg, 1.0f); // Bottom-right
    }

    // active bottom-indicator strip
    if (active) {
        dl->AddRectFilled(ImVec2(pos.x + 12, pos.y + size.y - 3), ImVec2(pos.x + size.x - 12, pos.y + size.y - 1), fg, 1.0f);
    }
    if (hovered && tooltip) {
        ImGui::SetTooltip("%s", tooltip);
    }
    return clicked;
}

// power button
static bool PowerButton(const char* id, const char* tooltip) {
    const ImVec2 size(44, 36);
    ImVec2 pos = ImGui::GetCursorScreenPos();
    bool clicked = ImGui::InvisibleButton(id, size);
    bool hovered = ImGui::IsItemHovered();
    ImDrawList* dl = ImGui::GetWindowDrawList();

    // Red glow states
    ImU32 bg = hovered ? IM_COL32(140, 30, 30, 255) : IM_COL32(40, 20, 25, 255);
    ImU32 fg = hovered ? IM_COL32(255, 120, 120, 255) : IM_COL32(235, 70, 70, 255);
    dl->AddRectFilled(pos, pos + size, bg, 6.0f);

    ImVec2 c = pos + size * 0.5f;
    const float radius = 8.5f;

    // Draw broken circle
    dl->PathArcTo(c, radius, -IM_PI * 0.30f, IM_PI * 1.30f, 24);
    dl->PathStroke(fg, 0, 2.2f);

    // Draw top vertical notch
    dl->AddLine(ImVec2(c.x, c.y - 10.0f), ImVec2(c.x, c.y - 1.0f), fg, 2.2f);

    if (hovered && tooltip) {
        ImGui::SetTooltip("%s", tooltip);
    }
    return clicked;
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


static void DrawTaskManager() {
    if (!shell.showTaskMgr) return;

    ImGui::SetNextWindowPos(ImVec2(100, 60), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(620, 480), ImGuiCond_FirstUseEver);

    if (ImGui::Begin("Task Manager", &shell.showTaskMgr)) {
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

        if (!shell.booted) {
            DrawBios(); // bios screen
        } else if (!shell.splashDone) {
            DrawSplash(); // splash screen
        } else {
            DrawDesktop(); // desktop layer
            DrawApps(); // applications
            DrawTaskbar(); // taskbar (+ PWR)
        }
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
