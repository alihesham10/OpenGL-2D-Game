#pragma once
#include <glm/glm.hpp>

class Player;
class Enemy;

enum class ProjectileOwner { Player, Enemy };
enum class ProjectileHeight { Upper, Lower };

class Projectile {
public:
    Projectile() = default;
    Projectile(glm::vec2 position, int lane, float speedPixelsPerSecond,
               int damage, ProjectileOwner owner,
               ProjectileHeight height = ProjectileHeight::Upper);

    void update(float deltaTime);
    bool crossedTarget(float previousX, float leftEdge, float rightEdge) const;
    bool isAvoidedBy(const Player& player) const;
    bool isOffscreen() const;

    glm::vec2 pos{0.0f};
    int lane = 0;
    float speedPixelsPerSecond = 0.0f;
    int damage = 1;
    ProjectileOwner owner = ProjectileOwner::Enemy;
    ProjectileHeight height = ProjectileHeight::Upper;
    bool alive = true;
};
