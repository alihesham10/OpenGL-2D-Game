#pragma once

enum class MenuState { None, Paused, GameOver, GameWon };

class Renderer;

class Menu {
public:
    void togglePause();
    void showGameOver();
    void showGameWon();
    void clear();
    MenuState state() const;
    void render(Renderer& renderer) const;

private:
    MenuState state_ = MenuState::None;
};
