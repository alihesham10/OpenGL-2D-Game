#pragma once

#include <glm/glm.hpp>

enum class EnemyType {
    EasyUpperRanged,
    MediumLowerRanged,
    HardMelee
};

struct Enemy {
    EnemyType type;
    glm::vec2 pos;
    int lane;
    float speed;
    int health;
    bool alive;
};
