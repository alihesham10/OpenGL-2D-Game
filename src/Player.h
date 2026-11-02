#pragma once

#include <glm/glm.hpp>

#include <vector>

class Enemy;
class Projectile;

enum class PlayerType { Melee, Defense, LongRange };

class Player {
public:
    Player() = default;
    Player(PlayerType type, glm::vec2 position, int lane);

    void update(float deltaTime, bool laneSelected, bool duckKeyHeld);
    void move(float deltaTime);
    void takeDamage(int damage);
    bool attackEnemy(Enemy& enemy, float deltaTime);
    void fireAtNearestEnemy(const std::vector<Enemy>& enemies,
                            std::vector<Projectile>& projectiles, float deltaTime);
    int maximumHealth() const;
    float bodyWidth() const;

    static PlayerType typeForCard(int card);
    static int costForCard(int card);
    static bool cardIsUnlocked(int card, int coins, int coinsEarnedSinceStart);

    PlayerType type = PlayerType::Melee;
    glm::vec2 pos{0.0f};
    int lane = 0;
    int health = 5;
    bool alive = true;
    float speedPixelsPerSecond = 0.0f;
    float jumpTimer = 0.0f;
    bool ducking = false;
    float attackCooldown = 0.0f;
    float attackAnimationTimer = 0.0f;
    int engagedEnemy = -1;
};
