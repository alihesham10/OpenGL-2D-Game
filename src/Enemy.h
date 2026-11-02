#pragma once

#include <glm/glm.hpp>

#include <vector>

class Player;
class Projectile;

enum class EnemyType {
    EasyUpperRanged,
    MediumLowerRanged,
    HardMelee
};

class Enemy {
public:
    Enemy() = default;
    Enemy(EnemyType type, glm::vec2 position, int lane);

    void moveLeft(float deltaTime);
    void takeDamage(int damage);
    int attack(std::vector<Player>& players, std::vector<Projectile>& projectiles,
               float deltaTime);
    int maximumHealth() const;
    float bodyWidth() const;
    float attackInterval() const;
    float projectileSpeed() const;

    EnemyType type;
    glm::vec2 pos;
    int lane;
    float speedPixelsPerSecond;
    int health;
    bool alive;
    bool engaged;
    float attackCooldown;
};
