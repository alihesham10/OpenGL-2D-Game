# Temple Defense

Temple Defense is a 2D OpenGL game project written in C++.

## Project layout

```text
Graphics_Project_Team_Rakam1/
├── CMakeLists.txt
├── .gitignore
├── README.md
├── main.cpp                 # Existing CMake entry point; includes src/main.cpp
├── src/
│   ├── main.cpp              # Current application implementation
│   ├── Game.h / Game.cpp
│   ├── Renderer.h / Renderer.cpp
│   ├── Player.h / Player.cpp
│   ├── Enemy.h / Enemy.cpp
│   ├── Projectile.h / Projectile.cpp
│   ├── Collectible.h / Collectible.cpp
│   ├── WaveManager.h / WaveManager.cpp
│   ├── UI.h / UI.cpp
│   └── Menu.h / Menu.cpp
├── external/
│   ├── glad/
│   ├── glfw-3.5.1/
│   └── glm/
└── build/                    # Generated locally; ignored by Git
```

The root `main.cpp` remains the source file named by the existing CMake
target. It includes `src/main.cpp`, so the project can keep using the
current build configuration. The other files in `src/` are scaffolds for
future extraction from the current application implementation.

## Build and run

From the project root in PowerShell:

```powershell
cmake --build build --config Debug
.\build\Debug\OpenGLStarter.exe
```

The source files under `src/` are not added to the CMake target yet. Add
them to the target when their code is ready to be split into separate
translation units.
