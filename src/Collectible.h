#pragma once
#include <glm/glm.hpp>

class Player;

class Collectible {
public:
    Collectible() = default;
    Collectible(glm::vec2 position, int lane, int value);
    bool collectIfTouched(const Player& player);

    glm::vec2 pos{0.0f};
    int lane = 0;
    int value = 0;
    bool alive = true;
};
