#include "Menu.h"

#include "Renderer.h"

void Menu::togglePause() {
    state_ = state_ == MenuState::Paused ? MenuState::None : MenuState::Paused;
}

void Menu::showGameOver() { state_ = MenuState::GameOver; }
void Menu::showGameWon() { state_ = MenuState::GameWon; }
void Menu::clear() { state_ = MenuState::None; }
MenuState Menu::state() const { return state_; }

void Menu::render(Renderer& renderer) const {
    if (state_ == MenuState::None) return;
    renderer.drawRectangle(0.0f, 0.0f, 0.90f, 0.44f, glm::vec3(0.10f, 0.11f, 0.13f));
    glm::vec3 statusColor = state_ == MenuState::GameWon ? glm::vec3(0.18f, 0.72f, 0.28f)
        : (state_ == MenuState::GameOver ? glm::vec3(0.78f, 0.18f, 0.15f)
                                        : glm::vec3(0.95f, 0.75f, 0.18f));
    renderer.drawRectangle(0.0f, 0.08f, 0.54f, 0.06f, statusColor);
    renderer.drawRectangle(0.0f, -0.08f, 0.34f, 0.04f, glm::vec3(0.85f));
}
