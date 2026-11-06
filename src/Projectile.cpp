#include "Projectile.h"
#include "GameConfig.h"
#include "Player.h"

Projectile::Projectile(glm::vec2 position, int projectileLane, float speed,int projectileDamage, ProjectileOwner projectileOwner,ProjectileHeight projectileHeight)
    : pos(position), lane(projectileLane), speedPixelsPerSecond(speed), damage(projectileDamage), owner(projectileOwner), height(projectileHeight) {}

void Projectile::update(float deltaTime) {
    float direction = -1.0f;
    if (owner == ProjectileOwner::Player) direction = 1.0f;
    pos.x += direction * 2.0f * speedPixelsPerSecond / GameConfig::WINDOW_W * deltaTime;
}

bool Projectile::crossedTarget(float previousX, float leftEdge, float rightEdge) const {
    if (owner == ProjectileOwner::Player)
        return previousX <= rightEdge && pos.x >= leftEdge;
    return previousX >= leftEdge && pos.x <= rightEdge;
}

bool Projectile::isAvoidedBy(const Player& player) const {
    if (player.type == PlayerType::Defense) return false;
    if (height == ProjectileHeight::Upper) return player.ducking;
    return player.jumpTimer > 0.0f;
}

bool Projectile::isOffscreen() const {
    if (owner == ProjectileOwner::Player) return pos.x > 1.1f;
    return pos.x < -1.1f;
}
