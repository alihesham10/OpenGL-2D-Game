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
