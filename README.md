# Temple Defense

Temple Defense is a 2D OpenGL game project written in C++.

## Project layout

```text
Graphics_Project_Team_Rakam1/
|-- CMakeLists.txt
|-- main.cpp                  # Constructs Game and calls run()
|-- src/
|   |-- Game.h / Game.cpp
|   |-- Renderer.h / Renderer.cpp
|   |-- Player.h / Player.cpp
|   |-- Enemy.h / Enemy.cpp
|   |-- Projectile.h / Projectile.cpp
|   |-- Collectible.h / Collectible.cpp
|   |-- WaveManager.h / WaveManager.cpp
|   |-- UI.h / UI.cpp
|   |-- Menu.h / Menu.cpp
|   `-- GameConfig.h
|-- external/
|   |-- glad/
|   |-- glfw-3.5.1/
|   `-- glm/
`-- build/                    # Generated locally; ignored by Git
```

The root `main.cpp` constructs a `Game` and calls `game.run()`. CMake compiles each implementation file in `src/` as a separate translation unit; implementation files are not included in other source files.

## Build and run

From the project root in PowerShell:

```powershell
cmake --build build --config Debug
.\build\Debug\OpenGLStarter.exe
```