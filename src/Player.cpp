#include "Player.h"

#include "GameConfig.h"
#include "Enemy.h"
#include "Projectile.h"

#include <algorithm>

Player::Player(PlayerType playerType, glm::vec2 position, int playerLane)
    : type(playerType), pos(position), lane(playerLane) {
    if (type == PlayerType::Defense) {
        health = 7;
        speedPixelsPerSecond = GameConfig::DEFENSE_PLAYER_SPEED_PIXELS_PER_SECOND;
    } else if (type == PlayerType::LongRange) {
        health = 5;
        speedPixelsPerSecond = GameConfig::LONG_RANGE_PLAYER_SPEED_PIXELS_PER_SECOND;
    } else {
        health = 5;
        speedPixelsPerSecond = GameConfig::MELEE_PLAYER_SPEED_PIXELS_PER_SECOND;
    }
}

void Player::update(float deltaTime, bool laneSelected, bool duckKeyHeld) {
    if (!alive) return;
    jumpTimer = std::max(0.0f, jumpTimer - deltaTime);
    attackAnimationTimer = std::max(0.0f, attackAnimationTimer - deltaTime);
    ducking = type != PlayerType::Defense && jumpTimer <= 0.0f && laneSelected && duckKeyHeld;
}

void Player::move(float deltaTime) {
    if (!alive || type == PlayerType::Defense || engagedEnemy >= 0) return;
    const float ndcSpeed = 2.0f * speedPixelsPerSecond / GameConfig::WINDOW_W;
    pos.x = std::min(GameConfig::PLAYER_RIGHT_LIMIT, pos.x + ndcSpeed * deltaTime);
}

void Player::takeDamage(int damage) {
    if (!alive) return;
    health -= damage;
    if (health <= 0) alive = false;
}

bool Player::attackEnemy(Enemy& enemy, float deltaTime) {
    if (type != PlayerType::Melee) return false;
    attackCooldown -= deltaTime;
    if (attackCooldown > 0.0f) return false;
    enemy.takeDamage(GameConfig::MELEE_PLAYER_DAMAGE);
    attackCooldown = GameConfig::MELEE_PLAYER_ATTACK_INTERVAL;
    attackAnimationTimer = GameConfig::MELEE_ATTACK_ANIMATION_DURATION;
    return enemy.health <= 0;
}

void Player::fireAtNearestEnemy(const std::vector<Enemy>& enemies,
                                std::vector<Projectile>& projectiles, float deltaTime) {
    if (!alive || type != PlayerType::LongRange) return;
    attackCooldown -= deltaTime;
    if (attackCooldown > 0.0f) return;
    int targetEnemy = -1;
    float nearestDistance = 100.0f;
    for (int i = 0; i < static_cast<int>(enemies.size()); ++i) {
        const Enemy& enemy = enemies[i];
        if (!enemy.alive || enemy.lane != lane || enemy.pos.x < pos.x) continue;
        const float distance = enemy.pos.x - pos.x;
        if (distance < nearestDistance) {
            nearestDistance = distance;
            targetEnemy = i;
        }
    }
    if (targetEnemy < 0) return;
    projectiles.emplace_back(glm::vec2(pos.x + 0.11f, GameConfig::LANE_Y[lane] + 0.015f),
                             lane, GameConfig::PLAYER_PROJECTILE_SPEED_PIXELS_PER_SECOND,
                             GameConfig::LONG_RANGE_PLAYER_DAMAGE, ProjectileOwner::Player);
    attackCooldown = GameConfig::LONG_RANGE_PLAYER_ATTACK_INTERVAL;
    attackAnimationTimer = GameConfig::MELEE_ATTACK_ANIMATION_DURATION;
}

int Player::maximumHealth() const {
    return type == PlayerType::Defense ? 7 : 5;
}

float Player::bodyWidth() const {
    if (type == PlayerType::Defense) return GameConfig::DEFENSE_BODY_WIDTH_NDC;
    if (type == PlayerType::LongRange) return GameConfig::LONG_RANGE_BODY_WIDTH_NDC;
    return GameConfig::MELEE_BODY_WIDTH_NDC;
}

PlayerType Player::typeForCard(int card) {
    if (card == 1) return PlayerType::LongRange;
    if (card == 2) return PlayerType::Defense;
    return PlayerType::Melee;
}

int Player::costForCard(int card) {
    if (card == 1) return GameConfig::LONG_RANGE_PLAYER_COST;
    if (card == 2) return GameConfig::DEFENSE_PLAYER_COST;
    return GameConfig::MELEE_PLAYER_COST;
}

bool Player::cardIsUnlocked(int card, int coins, int coinsEarnedSinceStart) {
    if (card == 0) return coins >= GameConfig::MELEE_UNLOCK_THRESHOLD;
    if (card == 1)
        return coinsEarnedSinceStart >= GameConfig::LONG_RANGE_UNLOCK_THRESHOLD &&
               coins >= GameConfig::LONG_RANGE_UNLOCK_THRESHOLD;
    if (card == 2)
        return coinsEarnedSinceStart >= GameConfig::DEFENSE_UNLOCK_THRESHOLD &&
               coins >= GameConfig::DEFENSE_UNLOCK_THRESHOLD;
    return false;
}
