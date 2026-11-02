#include "Enemy.h"

#include "GameConfig.h"
#include "Player.h"
#include "Projectile.h"

#include <cmath>

Enemy::Enemy(EnemyType enemyType, glm::vec2 position, int enemyLane)
    : type(enemyType), pos(position), lane(enemyLane), health(maximumHealth()),
      alive(true), engaged(false), attackCooldown(0.0f) {
    if (type == EnemyType::EasyUpperRanged)
        speedPixelsPerSecond = GameConfig::EASY_ENEMY_SPEED_PIXELS_PER_SECOND;
    else if (type == EnemyType::MediumLowerRanged)
        speedPixelsPerSecond = GameConfig::MEDIUM_ENEMY_SPEED_PIXELS_PER_SECOND;
    else
        speedPixelsPerSecond = GameConfig::HARD_ENEMY_SPEED_PIXELS_PER_SECOND;
}

void Enemy::moveLeft(float deltaTime) {
    pos.x -= 2.0f * speedPixelsPerSecond / GameConfig::WINDOW_W * deltaTime;
}

void Enemy::takeDamage(int damage) {
    if (!alive) return;
    health -= damage;
}

int Enemy::attack(std::vector<Player>& players, std::vector<Projectile>& projectiles,
                  float deltaTime) {
    Player* target = nullptr;
    float nearestDistance = 100.0f;
    int targetIndex = -1;
    for (int i = 0; i < static_cast<int>(players.size()); ++i) {
        Player& player = players[i];
        if (!player.alive || player.lane != lane) continue;
        const float distance = std::abs(pos.x - player.pos.x);
        if (distance < nearestDistance) {
            nearestDistance = distance;
            target = &player;
            targetIndex = i;
        }
    }
    if (!target) return -1;
    attackCooldown -= deltaTime;
    if (attackCooldown > 0.0f) return -1;

    if (type == EnemyType::HardMelee) {
        float contactRange = 0.5f * (target->bodyWidth() + bodyWidth());
        if (target->type == PlayerType::Melee)
            contactRange += GameConfig::MELEE_WEAPON_REACH_NDC;
        if (nearestDistance <= contactRange) {
            target->takeDamage(GameConfig::HARD_ENEMY_DAMAGE);
            attackCooldown = attackInterval();
            if (!target->alive) return targetIndex;
        }
        return -1;
    }

    const bool lowerAttack = type == EnemyType::MediumLowerRanged;
    const float heightOffset = lowerAttack ? -0.12f : 0.12f;
    projectiles.emplace_back(glm::vec2(pos.x - 0.04f, GameConfig::LANE_Y[lane] + heightOffset),
                             lane, projectileSpeed(),
                             lowerAttack ? GameConfig::MEDIUM_ENEMY_DAMAGE : GameConfig::EASY_ENEMY_DAMAGE,
                             ProjectileOwner::Enemy,
                             lowerAttack ? ProjectileHeight::Lower : ProjectileHeight::Upper);
    attackCooldown = attackInterval();
    return -1;
}

int Enemy::maximumHealth() const {
    return type == EnemyType::HardMelee ? 7 : 5;
}

float Enemy::bodyWidth() const {
    if (type == EnemyType::MediumLowerRanged) return GameConfig::MEDIUM_ENEMY_BODY_WIDTH_NDC;
    if (type == EnemyType::HardMelee) return GameConfig::HARD_ENEMY_BODY_WIDTH_NDC;
    return GameConfig::EASY_ENEMY_BODY_WIDTH_NDC;
}

float Enemy::attackInterval() const {
    if (type == EnemyType::MediumLowerRanged) return GameConfig::MEDIUM_ENEMY_ATTACK_INTERVAL;
    if (type == EnemyType::HardMelee) return GameConfig::HARD_ENEMY_ATTACK_INTERVAL;
    return GameConfig::EASY_ENEMY_ATTACK_INTERVAL;
}

float Enemy::projectileSpeed() const {
    if (type == EnemyType::MediumLowerRanged)
        return GameConfig::MEDIUM_ENEMY_PROJECTILE_SPEED_PIXELS_PER_SECOND;
    return GameConfig::EASY_ENEMY_PROJECTILE_SPEED_PIXELS_PER_SECOND;
}
