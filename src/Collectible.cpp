#include "Collectible.h"

#include "Player.h"

#include <cmath>

Collectible::Collectible(glm::vec2 position, int collectibleLane, int coinValue)
    : pos(position), lane(collectibleLane), value(coinValue), alive(true) {}

bool Collectible::collectIfTouched(const Player& player) {
    if (!alive || !player.alive || player.lane != lane) return false;
    const float collisionDistance = 0.5f * player.bodyWidth() + 0.04f;
    if (std::abs(player.pos.x - pos.x) > collisionDistance) return false;
    alive = false;
    return true;
}
