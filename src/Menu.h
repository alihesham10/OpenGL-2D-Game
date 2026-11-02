#pragma once

enum class MenuState { MainMenu, Cutscene, None, Paused, GameOver, GameWon };

class Renderer;

class Menu {
public:
    void showMainMenu();
    void showCutscene();
    void setHoveredMainOption(int option);
    int mainMenuOptionAt(float x, float y) const;
    void setHoveredPauseOption(int option);
    int pauseMenuOptionAt(float x, float y) const;
    void setHoveredResultOption(int option);
    int resultMenuOptionAt(float x, float y) const;
    void togglePause();
    void showGameOver();
    void showGameWon();
    void clear();
    MenuState state() const;
    void render(Renderer& renderer) const;

private:
    MenuState state_ = MenuState::MainMenu;
    int hoveredMainOption_ = -1;
    int hoveredPauseOption_ = -1;
    int hoveredResultOption_ = -1;
};
