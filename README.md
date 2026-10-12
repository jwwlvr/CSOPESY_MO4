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
- CMake
- MinGW-w64 with GCC and G++
- MinGW Make
- Dear ImGui and GLFW dependencies required by the project
- OpenGL support

The required dependency paths must be correctly configured in `CMakeLists.txt`.

## Features

- **Desktop Interface** — Displays a graphical desktop with a gradient wallpaper.
- **Live Clock** — Shows the current time on the desktop interface.
- **Taskbar** — Provides a taskbar at the bottom of the desktop.
- **Power Control** — Includes a power button for initiating the shutdown flow.
- **Close Warning** — Displays a warning when the user attempts to close the application without using the power control.

## Technologies Used

- **C++** — Main programming language.
- **CMake** — Project configuration and build system.
- **Dear ImGui** — Graphical user interface.
- **GLFW** — Window creation and input handling.
- **OpenGL** — Graphics rendering.
- **MinGW-w64 / GCC** — C and C++ compiler toolchain.

## Building the Project

### 1. Open the Project

Open the `CSOPESY_MO4` folder in Visual Studio Code. Ensure that `CMakeLists.txt` is located in the project root.

### 2. Open a Terminal

In VS Code, select **Terminal → New Terminal** and navigate to the project directory.

### 3. Configure CMake

Run the following command:

```bash
cmake -S . -B build -G "MinGW Makefiles" -DCMAKE_MAKE_PROGRAM=C:/msys64/mingw64/bin/mingw32-make.exe -DCMAKE_C_COMPILER=C:/msys64/mingw64/bin/gcc.exe -DCMAKE_CXX_COMPILER=C:/msys64/mingw64/bin/g++.exe
```

This configures the project using the MinGW compiler toolchain.

### 4. Compile the Application

Run:

```bash
cmake --build build
```

Wait for the build to complete successfully. A successful build should report that the `main` target has been built.

### 5. Run the Application

Launch the compiled executable:

```powershell
.\build\main.exe
```

Alternatively, use the CMake Tools extension in VS Code to build and launch the configured `main` target.
