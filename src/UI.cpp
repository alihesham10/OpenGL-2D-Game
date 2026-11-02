#include "UI.h"

#include "GameConfig.h"
#include "Renderer.h"

#include <algorithm>
#include <string>

void UI::drawDigit(Renderer& renderer, int digit, float centerX, float centerY, bool grayscale) const {
    static const bool segments[10][7] = {
        {true, true, true, true, true, true, false},
        {false, true, true, false, false, false, false},
        {true, true, false, true, true, false, true},
        {true, true, true, true, false, false, true},
        {false, true, true, false, false, true, true},
        {true, false, true, true, false, true, true},
        {true, false, true, true, true, true, true},
        {true, true, true, false, false, false, false},
        {true, true, true, true, true, true, true},
        {true, true, true, true, false, true, true}
    };
    if (digit < 0 || digit > 9) return;
    const float gray = 0.299f + 0.587f * 0.86f + 0.114f * 0.35f;
    const glm::vec3 color = grayscale ? glm::vec3(gray) : glm::vec3(1.0f, 0.86f, 0.35f);
    const float horizontalWidth = 0.035f;
    const float horizontalHeight = 0.009f;
    const float verticalWidth = 0.009f;
    const float verticalHeight = 0.025f;
    const float topY = centerY + 0.030f;
    const float middleY = centerY;
    const float bottomY = centerY - 0.030f;
    const float leftX = centerX - 0.018f;
    const float rightX = centerX + 0.018f;
    const float segmentX[7] = {centerX, rightX, rightX, centerX, leftX, leftX, centerX};
    const float segmentY[7] = {topY, centerY + 0.015f, centerY - 0.015f,
                               bottomY, centerY - 0.015f, centerY + 0.015f, middleY};
    for (int segment = 0; segment < 7; ++segment) {
        if (!segments[digit][segment]) continue;
        const bool horizontal = segment == 0 || segment == 3 || segment == 6;
        renderer.drawRectangle(segmentX[segment], segmentY[segment],
                               horizontal ? horizontalWidth : verticalWidth,
                               horizontal ? horizontalHeight : verticalHeight, color);
    }
}

void UI::drawName(Renderer& renderer, int card, float centerX, float centerY,
                  bool grayscale) const {
    // 5-by-7 bitmap letters keep the button labels independent of any font library.
    static const char* const names[] = {"HESHAM", "FAHMY", "SHIKO"};
    static const char* const glyphs[26][7] = {
        {"01110","10001","10001","11111","10001","10001","10001"}, // A
        {"11110","10001","10001","11110","10001","10001","11110"}, // B
        {"01111","10000","10000","10000","10000","10000","01111"}, // C
        {"11110","10001","10001","10001","10001","10001","11110"}, // D
        {"11111","10000","10000","11110","10000","10000","11111"}, // E
        {"11111","10000","10000","11110","10000","10000","10000"}, // F
        {"01111","10000","10000","10111","10001","10001","01111"}, // G
        {"10001","10001","10001","11111","10001","10001","10001"}, // H
        {"11111","00100","00100","00100","00100","00100","11111"}, // I
        {"00111","00010","00010","00010","10010","10010","01100"}, // J
        {"10001","10010","10100","11000","10100","10010","10001"}, // K
        {"10000","10000","10000","10000","10000","10000","11111"}, // L
        {"10001","11011","10101","10101","10001","10001","10001"}, // M
        {"10001","11001","10101","10011","10001","10001","10001"}, // N
        {"01110","10001","10001","10001","10001","10001","01110"}, // O
        {"11110","10001","10001","11110","10000","10000","10000"}, // P
        {"01110","10001","10001","10001","10101","10010","01101"}, // Q
        {"11110","10001","10001","11110","10100","10010","10001"}, // R
        {"01111","10000","10000","01110","00001","00001","11110"}, // S
        {"11111","00100","00100","00100","00100","00100","00100"}, // T
        {"10001","10001","10001","10001","10001","10001","01110"}, // U
        {"10001","10001","10001","10001","10001","01010","00100"}, // V
        {"10001","10001","10001","10101","10101","10101","01010"}, // W
        {"10001","10001","01010","00100","01010","10001","10001"}, // X
        {"10001","10001","01010","00100","00100","00100","00100"}, // Y
        {"11111","00001","00010","00100","01000","10000","11111"}  // Z
    };
    if (card < 0 || card >= 3) return;
    const std::string name = names[card];
    constexpr float pixel = 0.0045f;
    constexpr float letterStep = pixel * 6.0f;
    constexpr float rowStep = pixel * 1.15f;
    const float firstX = centerX - (name.size() - 1) * letterStep * 0.5f;
    const float gray = 0.299f + 0.587f * 0.86f + 0.114f * 0.35f;
    const glm::vec3 color = grayscale ? glm::vec3(gray) : glm::vec3(1.0f, 0.86f, 0.35f);
    for (std::size_t letter = 0; letter < name.size(); ++letter) {
        const int glyphIndex = name[letter] - 'A';
        for (int row = 0; row < 7; ++row) {
            for (int column = 0; column < 5; ++column) {
                if (glyphs[glyphIndex][row][column] != '1') continue;
                const float x = firstX + letter * letterStep + (column - 2) * pixel;
                const float y = centerY + (3 - row) * rowStep;
                renderer.drawRectangle(x, y, pixel * 0.82f, pixel * 0.82f, color);
            }
        }
    }
}

void UI::drawCoinCount(Renderer& renderer, int coins) const {
    renderer.drawCircle(-0.84f, GameConfig::PLAYER_CARD_Y, 0.035f, 0.035f,
                        glm::vec3(1.0f, 0.78f, 0.10f));
    renderer.drawCircle(-0.84f, GameConfig::PLAYER_CARD_Y, 0.020f, 0.020f,
                        glm::vec3(0.90f, 0.52f, 0.05f));
    std::string digits = std::to_string(std::max(0, coins));
    if (digits.size() > 3) digits = digits.substr(digits.size() - 3);
    float startX = -0.76f + 0.065f * static_cast<float>(3 - digits.size());
    for (char digit : digits) {
        drawDigit(renderer, digit - '0', startX, GameConfig::PLAYER_CARD_Y);
        startX += 0.065f;
    }
}

void UI::drawPlayerCard(Renderer& renderer, int card, float centerX,
                        int coins, int coinsEarnedSinceStart,
                        int hoveredPlayerCard) const {
    const PlayerType type = Player::typeForCard(card);
    const bool unlocked = Player::cardIsUnlocked(card, coins, coinsEarnedSinceStart);
    const float scale = hoveredPlayerCard == card ? 0.52f : 0.48f;
    renderer.drawPlayerCharacter(centerX - 0.04f, GameConfig::PLAYER_CARD_Y, type,
                                 type == PlayerType::Defense ? 7 : 5, 0.0f, false,
                                 false, scale, !unlocked);
    const int cost = Player::costForCard(card);
    drawName(renderer, card, centerX, GameConfig::PLAYER_CARD_Y - 0.073f, !unlocked);
    drawDigit(renderer, cost / 10, centerX + 0.12f, GameConfig::PLAYER_CARD_Y, !unlocked);
    drawDigit(renderer, cost % 10, centerX + 0.175f, GameConfig::PLAYER_CARD_Y, !unlocked);
}

void UI::render(Renderer& renderer, int coins, int coinsEarnedSinceStart,
                int hoveredPlayerCard) const {
    renderer.drawRectangle(0.0f, GameConfig::PLAYER_CARD_Y, 1.90f, 0.20f,
                           glm::vec3(0.12f, 0.13f, 0.15f));
    for (int card = 0; card < 3; ++card) {
        drawPlayerCard(renderer, card, GameConfig::PLAYER_CARD_X[card],
                       coins, coinsEarnedSinceStart, hoveredPlayerCard);
    }
    drawCoinCount(renderer, coins);
}
