# CSOPESY_MO4

## Group Developers

- Austria, Ma. Alexandria
- De Leon, Sofia Ysabela
- Guererro, Laura Mae
- Patricio, Anne Beatriz

## Entry Class / Main Function

- **Entry file:** `main.cpp`

## Program Requirements

- Visual Studio Code
- CMake Tools extension for VS Code
- CMake
- MinGW-w64 with GCC and G++
- MinGW Make
- Dear ImGui and GLFW dependencies required by the project
- OpenGL support

The required dependency paths must be correctly configured in `CMakeLists.txt`.

## Features

- **Desktop Interface** — Displays a graphical desktop with a gradient wallpaper.
- **Live Clock** — Shows the current time on the desktop interface.
- **Taskbar** — Provides a taskbar at the bottom of the desktop with application icons.
- **Task Manager** — Simulated processes table with resource metrics and performance monitoring.
- **Notepad** — Text editor application accessible from the taskbar, allowing continuous multiline note taking and text editing in memory.
- **Calculator** — Fully interactive arithmetic calculator application featuring an LCD-style readout, numeric key inputs, basic operators (`+`, `-`, `*`, `/`), clear (`C`), and chained calculations.
- **Power Control** — Includes a power button for initiating the shutdown flow.
- **Close Warning** — Displays a warning when the user attempts to close the application without using the power control.

## Technologies Used

- **C++** — Main programming language.
- **CMake** — Project configuration and build system.
- **Dear ImGui** — Graphical user interface.
- **GLFW** — Window creation and input handling.
- **OpenGL** — Graphics rendering.
- **MinGW-w64 / GCC** — C and C++ compiler toolchain.

---

## Running the Program via VS Code (Recommended)

1. **Install Prerequisites**: Ensure the **CMake Tools** extension is installed in VS Code.
2. **Open Project**: Open the `CSOPESY_MO4` project folder in VS Code.
3. **Configure & Build**:
   - Press `Ctrl + Shift + P` to open the Command Palette.
   - Run `CMake: Reconfigure` or select your configured MinGW kit.
   - Run `CMake: Build`.
4. **Launch**: Click the **Launch / Play button** located on the bottom status bar (blue bar) to run the application.

---

## Alternative: Building & Running via Terminal

### 1. Open a Terminal
In VS Code, select **Terminal → New Terminal** and navigate to the project directory.

### 2. Configure CMake
Run the following command:

```bash
cmake -S . -B build -G "MinGW Makefiles" -DCMAKE_MAKE_PROGRAM=C:/msys64/mingw64/bin/mingw32-make.exe -DCMAKE_C_COMPILER=C:/msys64/mingw64/bin/gcc.exe -DCMAKE_CXX_COMPILER=C:/msys64/mingw64/bin/g++.exe