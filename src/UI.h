#pragma once

#include "Player.h"

class Renderer;

class UI {
public:
    void render(Renderer& renderer, int coins, int coinsEarnedSinceStart,
                int hoveredPlayerCard) const;

private:
    void drawDigit(Renderer& renderer, int digit, float centerX, float centerY,
                   bool grayscale = false) const;
    void drawCoinCount(Renderer& renderer, int coins) const;
    void drawPlayerCard(Renderer& renderer, int card, float centerX,
                        int coins, int coinsEarnedSinceStart,
                        int hoveredPlayerCard) const;
};
