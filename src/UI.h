#pragma once
#include "Player.h"
#include <string>

class Renderer;

class UI {
public:
    void render(Renderer& renderer, int coins, int coinsEarnedSinceStart, int hoveredPlayerCard) const;
    void renderCutsceneDialogue(Renderer& renderer, const std::string& speaker, const std::string& dialogue, const std::string& title = std::string()) const;

private:
    void drawDigit(Renderer& renderer, int digit, float centerX, float centerY, bool grayscale = false) const;
    void drawName(Renderer& renderer, int card, float centerX, float centerY, bool grayscale) const;
    void drawCoinCount(Renderer& renderer, int coins) const;
    void drawPlayerCard(Renderer& renderer, int card, float centerX, int coins, int coinsEarnedSinceStart, int hoveredPlayerCard) const;
};
