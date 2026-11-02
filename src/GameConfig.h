#pragma once

namespace GameConfig {
constexpr int WINDOW_W = 1100;
constexpr int WINDOW_H = 700;
constexpr int LANE_COUNT = 3;
constexpr float LANE_HEIGHT = 0.32f;
constexpr float LANE_Y[LANE_COUNT] = {-0.125f, -0.48f, -0.835f};
constexpr float PLAYER_CARD_X[3] = {-0.38f, 0.08f, 0.54f};
constexpr float PLAYER_CARD_Y = 0.90f;
constexpr float PLAYER_CARD_HALF_WIDTH = 0.21f;
constexpr float PLAYER_CARD_HALF_HEIGHT = 0.09f;

constexpr float EASY_ENEMY_SPEED_PIXELS_PER_SECOND = 30.0f;
constexpr float MEDIUM_ENEMY_SPEED_PIXELS_PER_SECOND = 35.0f;
constexpr float HARD_ENEMY_SPEED_PIXELS_PER_SECOND = 25.0f;
constexpr float MELEE_PLAYER_SPEED_PIXELS_PER_SECOND = 45.0f;
constexpr float LONG_RANGE_PLAYER_SPEED_PIXELS_PER_SECOND = 45.0f;
constexpr float DEFENSE_PLAYER_SPEED_PIXELS_PER_SECOND = 0.0f;
constexpr float EASY_ENEMY_ATTACK_INTERVAL = 2.0f;
constexpr float MEDIUM_ENEMY_ATTACK_INTERVAL = 1.5f;
constexpr float HARD_ENEMY_ATTACK_INTERVAL = 1.2f;
constexpr int EASY_ENEMY_DAMAGE = 1;
constexpr int MEDIUM_ENEMY_DAMAGE = 1;
constexpr int HARD_ENEMY_DAMAGE = 2;
constexpr int MELEE_PLAYER_DAMAGE = 1;
constexpr int LONG_RANGE_PLAYER_DAMAGE = 1;
constexpr float MELEE_PLAYER_ATTACK_INTERVAL = 1.0f;
constexpr float LONG_RANGE_PLAYER_ATTACK_INTERVAL = 1.5f;
constexpr float DEFENSE_ACTION_INTERVAL = 1.0f;
constexpr float PLAYER_PROJECTILE_SPEED_PIXELS_PER_SECOND = 300.0f;
constexpr float EASY_ENEMY_PROJECTILE_SPEED_PIXELS_PER_SECOND = 180.0f;
constexpr float MEDIUM_ENEMY_PROJECTILE_SPEED_PIXELS_PER_SECOND = 220.0f;
constexpr float ENEMY_SPAWN_INTERVAL = 3.0f;
constexpr float JUMP_DURATION = 0.55f;
constexpr float MELEE_ATTACK_ANIMATION_DURATION = 0.20f;
constexpr float MELEE_WEAPON_REACH_NDC = 0.08f;
constexpr float HOME_BASE_X = -0.78f;
constexpr float PLAYER_START_X = -0.66f;
constexpr float PLAYER_RIGHT_LIMIT = 0.80f;
constexpr float MELEE_BODY_WIDTH_NDC = 0.075f;
constexpr float DEFENSE_BODY_WIDTH_NDC = 0.14f;
constexpr float LONG_RANGE_BODY_WIDTH_NDC = 0.065f;
constexpr float EASY_ENEMY_BODY_WIDTH_NDC = 0.075f;
constexpr float MEDIUM_ENEMY_BODY_WIDTH_NDC = 0.13f;
constexpr float HARD_ENEMY_BODY_WIDTH_NDC = 0.14f;

constexpr int STARTING_COINS = 30;
constexpr int ENEMY_COIN_REWARD = 10;
constexpr int MELEE_PLAYER_COST = 10;
constexpr int LONG_RANGE_PLAYER_COST = 20;
constexpr int DEFENSE_PLAYER_COST = 30;
constexpr int MELEE_UNLOCK_THRESHOLD = 10;
constexpr int LONG_RANGE_UNLOCK_THRESHOLD = 20;
constexpr int DEFENSE_UNLOCK_THRESHOLD = 30;
}
