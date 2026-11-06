#include "Menu.h"

#include "Renderer.h"

#include <string>

namespace {
const char* glyph(char letter) {
    switch (letter) {
    case 'A': return "01110/10001/10001/11111/10001/10001/10001";
    case 'C': return "01111/10000/10000/10000/10000/10000/01111";
    case 'D': return "11110/10001/10001/10001/10001/10001/11110";
    case 'E': return "11111/10000/10000/11110/10000/10000/11111";
    case 'F': return "11111/10000/10000/11110/10000/10000/10000";
    case 'G': return "01111/10000/10000/10111/10001/10001/01111";
    case 'H': return "10001/10001/10001/11111/10001/10001/10001";
    case 'I': return "11111/00100/00100/00100/00100/00100/11111";
    case 'L': return "10000/10000/10000/10000/10000/10000/11111";
    case 'M': return "10001/11011/10101/10101/10001/10001/10001";
    case 'N': return "10001/11001/10101/10011/10001/10001/10001";
    case 'O': return "01110/10001/10001/10001/10001/10001/01110";
    case 'P': return "11110/10001/10001/11110/10000/10000/10000";
    case 'R': return "11110/10001/10001/11110/10100/10010/10001";
    case 'S': return "01111/10000/10000/01110/00001/00001/11110";
    case 'T': return "11111/00100/00100/00100/00100/00100/00100";
    case 'U': return "10001/10001/10001/10001/10001/10001/01110";
    case 'W': return "10001/10001/10001/10101/10101/10101/01010";
    case 'X': return "10001/10001/01010/00100/01010/10001/10001";
    case 'Y': return "10001/10001/01010/00100/00100/00100/00100";
    case '2': return "01110/10001/00001/00010/00100/01000/11111";
    default: return nullptr;
    }
}

void drawText(Renderer& renderer, const std::string& text, float centerX, float centerY, float pixelSize, glm::vec3 color) {
    const float letterStep = pixelSize * 6.0f;
    const float firstX = centerX - (static_cast<float>(text.size()) - 1.0f) * letterStep * 0.5f;
    for (std::size_t letterIndex = 0; letterIndex < text.size(); ++letterIndex) {
        const char* rows = glyph(text[letterIndex]);
        if (!rows) continue;
        for (int row = 0; row < 7; ++row) {
            const char* rowStart = rows + row * 6;
            for (int column = 0; column < 5; ++column) {
                if (rowStart[column] != '1') continue;
                const float x = firstX + static_cast<float>(letterIndex) * letterStep + (column - 2) * pixelSize;
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
    hoveredPauseOption_ = -1;
    hoveredResultOption_ = -1;
}

void Menu::showCutscene() {
    state_ = MenuState::Cutscene;
    hoveredMainOption_ = -1;
    hoveredPauseOption_ = -1;
    hoveredResultOption_ = -1;
}

void Menu::setHoveredMainOption(int option) {
    if (state_ == MenuState::MainMenu && (option == 0 || option == 1)) {
        hoveredMainOption_ = option;
    } else {
        hoveredMainOption_ = -1;
    }
}

int Menu::mainMenuOptionAt(float x, float y) const {
    if (state_ != MenuState::MainMenu || x < -0.38f || x > 0.38f) return -1;
    if (y >= 0.00f && y <= 0.16f) return 0;
    if (y >= -0.23f && y <= -0.07f) return 1;
    return -1;
}

void Menu::setHoveredPauseOption(int option) {
    if (state_ == MenuState::Paused && option >= 0 && option <= 2) {
        hoveredPauseOption_ = option;
    } else {
        hoveredPauseOption_ = -1;
    }
}

int Menu::pauseMenuOptionAt(float x, float y) const {
    if (state_ != MenuState::Paused || x < -0.40f || x > 0.40f) return -1;
    if (y >= 0.12f && y <= 0.28f) return 0;
    if (y >= -0.10f && y <= 0.06f) return 1;
    if (y >= -0.32f && y <= -0.16f) return 2;
    return -1;
}

void Menu::setHoveredResultOption(int option) {
    const bool showingResult = state_ == MenuState::GameOver || state_ == MenuState::GameWon;
    if (showingResult && (option == 0 || option == 1)) {
        hoveredResultOption_ = option;
    } else {
        hoveredResultOption_ = -1;
    }
}

int Menu::resultMenuOptionAt(float x, float y) const {
    if ((state_ != MenuState::GameOver && state_ != MenuState::GameWon) || x < -0.40f || x > 0.40f) return -1;
    if (y >= -0.02f && y <= 0.14f) return 0;
    if (y >= -0.24f && y <= -0.08f) return 1;
    return -1;
}

void Menu::togglePause() {
    if (state_ == MenuState::Paused) {
        state_ = MenuState::None;
    } else {
        state_ = MenuState::Paused;
    }
}

void Menu::showGameOver() {
    state_ = MenuState::GameOver;
    hoveredResultOption_ = -1;
}
void Menu::showGameWon() {
    state_ = MenuState::GameWon;
    hoveredResultOption_ = -1;
}
void Menu::clear() {
    state_ = MenuState::None;
    hoveredMainOption_ = -1;
    hoveredPauseOption_ = -1;
    hoveredResultOption_ = -1;
}
MenuState Menu::state() const { return state_; }

void Menu::render(Renderer& renderer) const {
    if (state_ == MenuState::MainMenu) {
        renderer.drawRectangle(0.0f, 0.0f, 1.88f, 1.78f, glm::vec3(0.09f, 0.11f, 0.13f));
        renderer.drawRectangle(0.0f, 0.0f, 1.78f, 1.68f, glm::vec3(0.16f, 0.20f, 0.20f));
        renderer.drawRectangle(0.0f, 0.57f, 0.92f, 0.014f, glm::vec3(0.74f, 0.52f, 0.22f));
        drawText(renderer, "WARA2ET SHAFRA", 0.0f, 0.43f, 0.011f, glm::vec3(0.98f, 0.82f, 0.43f));

        glm::vec3 startColor(0.23f, 0.39f, 0.27f);
        if (hoveredMainOption_ == 0) startColor = glm::vec3(0.38f, 0.60f, 0.34f);
        glm::vec3 exitColor(0.43f, 0.25f, 0.20f);
        if (hoveredMainOption_ == 1) exitColor = glm::vec3(0.68f, 0.36f, 0.24f);
        renderer.drawRectangle(0.0f, 0.08f, 0.78f, 0.18f, startColor);
        renderer.drawRectangle(0.0f, -0.15f, 0.78f, 0.18f, exitColor);
        drawText(renderer, "START GAME", 0.0f, 0.08f, 0.008f, glm::vec3(1.0f, 0.94f, 0.78f));
        drawText(renderer, "EXIT GAME", 0.0f, -0.15f, 0.008f, glm::vec3(1.0f, 0.94f, 0.78f));
        return;
    }

    if (state_ == MenuState::Paused) {
        renderer.drawRectangle(0.0f, 0.0f, 1.08f, 0.92f, glm::vec3(0.10f, 0.12f, 0.14f));
        drawText(renderer, "PAUSED", 0.0f, 0.34f, 0.010f, glm::vec3(0.98f, 0.82f, 0.43f));
        const float buttonY[3] = {0.20f, -0.02f, -0.24f};
        const char* labels[3] = {"CONTINUE", "RESTART", "MAIN MENU"};
        const glm::vec3 normalColors[3] = {
            glm::vec3(0.23f, 0.39f, 0.27f),
            glm::vec3(0.43f, 0.34f, 0.18f),
            glm::vec3(0.43f, 0.25f, 0.20f)
        };
        const glm::vec3 hoverColors[3] = {
            glm::vec3(0.38f, 0.60f, 0.34f),
            glm::vec3(0.65f, 0.50f, 0.22f),
            glm::vec3(0.68f, 0.36f, 0.24f)
        };
        for (int option = 0; option < 3; ++option) {
            glm::vec3 buttonColor = normalColors[option];
            if (hoveredPauseOption_ == option) buttonColor = hoverColors[option];
            renderer.drawRectangle(0.0f, buttonY[option], 0.82f, 0.17f, buttonColor);
            drawText(renderer, labels[option], 0.0f, buttonY[option], 0.0075f,
                     glm::vec3(1.0f, 0.94f, 0.78f));
        }
        return;
    }

    if (state_ == MenuState::None) return;
    const bool won = state_ == MenuState::GameWon;
    const bool lost = state_ == MenuState::GameOver;
    if (won || lost) {
        renderer.drawRectangle(0.0f, 0.0f, 1.12f, 0.76f, glm::vec3(0.10f, 0.11f, 0.13f));
        const char* resultText = "YOU LOSE";
        glm::vec3 resultTextColor(0.95f, 0.30f, 0.24f);
        if (won) {
            resultText = "YOU WIN";
            resultTextColor = glm::vec3(0.35f, 0.90f, 0.40f);
        }
        drawText(renderer, resultText, 0.0f, 0.27f, 0.010f, resultTextColor);
        glm::vec3 restartColor(0.43f, 0.34f, 0.18f);
        if (hoveredResultOption_ == 0) restartColor = glm::vec3(0.65f, 0.50f, 0.22f);
        glm::vec3 mainMenuColor(0.23f, 0.39f, 0.27f);
        if (hoveredResultOption_ == 1) mainMenuColor = glm::vec3(0.38f, 0.60f, 0.34f);
        renderer.drawRectangle(0.0f, 0.06f, 0.82f, 0.17f, restartColor);
        renderer.drawRectangle(0.0f, -0.16f, 0.82f, 0.17f, mainMenuColor);
        drawText(renderer, "RESTART", 0.0f, 0.06f, 0.0075f, glm::vec3(1.0f, 0.94f, 0.78f));
        drawText(renderer, "MAIN MENU", 0.0f, -0.16f, 0.0075f, glm::vec3(1.0f, 0.94f, 0.78f));
        return;
    }

    renderer.drawRectangle(0.0f, 0.0f, 0.90f, 0.44f, glm::vec3(0.10f, 0.11f, 0.13f));
    renderer.drawRectangle(0.0f, 0.08f, 0.54f, 0.06f, glm::vec3(0.95f, 0.75f, 0.18f));
    renderer.drawRectangle(0.0f, -0.08f, 0.34f, 0.04f, glm::vec3(0.85f));
}
