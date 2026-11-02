#include "Menu.h"

#include "Renderer.h"

#include <string>

namespace {
const char* glyph(char letter) {
    switch (letter) {
    case 'A': return "01110/10001/10001/11111/10001/10001/10001";
    case 'D': return "11110/10001/10001/10001/10001/10001/11110";
    case 'E': return "11111/10000/10000/11110/10000/10000/11111";
    case 'F': return "11111/10000/10000/11110/10000/10000/10000";
    case 'G': return "01111/10000/10000/10111/10001/10001/01111";
    case 'I': return "11111/00100/00100/00100/00100/00100/11111";
    case 'L': return "10000/10000/10000/10000/10000/10000/11111";
    case 'M': return "10001/11011/10101/10101/10001/10001/10001";
    case 'N': return "10001/11001/10101/10011/10001/10001/10001";
    case 'P': return "11110/10001/10001/11110/10000/10000/10000";
    case 'R': return "11110/10001/10001/11110/10100/10010/10001";
    case 'S': return "01111/10000/10000/01110/00001/00001/11110";
    case 'T': return "11111/00100/00100/00100/00100/00100/00100";
    case 'X': return "10001/10001/01010/00100/01010/10001/10001";
    default: return nullptr;
    }
}

void drawText(Renderer& renderer, const std::string& text, float centerX, float centerY,
              float pixelSize, glm::vec3 color) {
    const float letterStep = pixelSize * 6.0f;
    const float firstX = centerX - (static_cast<float>(text.size()) - 1.0f) * letterStep * 0.5f;
    for (std::size_t letterIndex = 0; letterIndex < text.size(); ++letterIndex) {
        const char* rows = glyph(text[letterIndex]);
        if (!rows) continue;
        for (int row = 0; row < 7; ++row) {
            const char* rowStart = rows + row * 6;
            for (int column = 0; column < 5; ++column) {
                if (rowStart[column] != '1') continue;
                const float x = firstX + static_cast<float>(letterIndex) * letterStep +
                    (column - 2) * pixelSize;
                const float y = centerY + (3 - row) * pixelSize;
                renderer.drawRectangle(x, y, pixelSize * 0.82f, pixelSize * 0.82f, color);
            }
        }
    }
}
}

void Menu::showMainMenu() {
    state_ = MenuState::MainMenu;
    hoveredMainOption_ = -1;
}

void Menu::setHoveredMainOption(int option) {
    hoveredMainOption_ = state_ == MenuState::MainMenu && (option == 0 || option == 1)
        ? option : -1;
}

int Menu::mainMenuOptionAt(float x, float y) const {
    if (state_ != MenuState::MainMenu || x < -0.38f || x > 0.38f) return -1;
    if (y >= 0.00f && y <= 0.16f) return 0;
    if (y >= -0.23f && y <= -0.07f) return 1;
    return -1;
}

void Menu::togglePause() {
    state_ = state_ == MenuState::Paused ? MenuState::None : MenuState::Paused;
}

void Menu::showGameOver() { state_ = MenuState::GameOver; }
void Menu::showGameWon() { state_ = MenuState::GameWon; }
void Menu::clear() { state_ = MenuState::None; hoveredMainOption_ = -1; }
MenuState Menu::state() const { return state_; }

void Menu::render(Renderer& renderer) const {
    if (state_ == MenuState::MainMenu) {
        renderer.drawRectangle(0.0f, 0.0f, 1.88f, 1.78f, glm::vec3(0.09f, 0.11f, 0.13f));
        renderer.drawRectangle(0.0f, 0.0f, 1.78f, 1.68f, glm::vec3(0.16f, 0.20f, 0.20f));
        renderer.drawRectangle(0.0f, 0.57f, 0.92f, 0.014f, glm::vec3(0.74f, 0.52f, 0.22f));
        drawText(renderer, "TEMPLE DEFENSE", 0.0f, 0.43f, 0.012f,
                 glm::vec3(0.98f, 0.82f, 0.43f));

        const glm::vec3 startColor = hoveredMainOption_ == 0
            ? glm::vec3(0.38f, 0.60f, 0.34f) : glm::vec3(0.23f, 0.39f, 0.27f);
        const glm::vec3 exitColor = hoveredMainOption_ == 1
            ? glm::vec3(0.68f, 0.36f, 0.24f) : glm::vec3(0.43f, 0.25f, 0.20f);
        renderer.drawRectangle(0.0f, 0.08f, 0.78f, 0.18f, startColor);
        renderer.drawRectangle(0.0f, -0.15f, 0.78f, 0.18f, exitColor);
        drawText(renderer, "START GAME", 0.0f, 0.08f, 0.008f, glm::vec3(1.0f, 0.94f, 0.78f));
        drawText(renderer, "EXIT GAME", 0.0f, -0.15f, 0.008f, glm::vec3(1.0f, 0.94f, 0.78f));
        return;
    }

    if (state_ == MenuState::None) return;
    renderer.drawRectangle(0.0f, 0.0f, 0.90f, 0.44f, glm::vec3(0.10f, 0.11f, 0.13f));
    const glm::vec3 statusColor = state_ == MenuState::GameWon ? glm::vec3(0.18f, 0.72f, 0.28f)
        : (state_ == MenuState::GameOver ? glm::vec3(0.78f, 0.18f, 0.15f)
                                        : glm::vec3(0.95f, 0.75f, 0.18f));
    renderer.drawRectangle(0.0f, 0.08f, 0.54f, 0.06f, statusColor);
    renderer.drawRectangle(0.0f, -0.08f, 0.34f, 0.04f, glm::vec3(0.85f));
}
