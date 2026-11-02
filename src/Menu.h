#pragma once

enum class MenuState { MainMenu, None, Paused, GameOver, GameWon };

class Renderer;

class Menu {
public:
    void showMainMenu();
    void setHoveredMainOption(int option);
    int mainMenuOptionAt(float x, float y) const;
    void togglePause();
    void showGameOver();
    void showGameWon();
    void clear();
    MenuState state() const;
    void render(Renderer& renderer) const;

private:
    MenuState state_ = MenuState::MainMenu;
    int hoveredMainOption_ = -1;
};
