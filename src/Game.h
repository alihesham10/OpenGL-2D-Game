#pragma once

#include "Collectible.h"
#include "Enemy.h"
#include "Menu.h"
#include "Player.h"
#include "Projectile.h"
#include "Renderer.h"
#include "UI.h"
#include "WaveManager.h"

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <vector>

class Game {
public:
    int run();

private:
    static void cursorPositionCallback(GLFWwindow* window, double x, double y);
    static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
    static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

    void handleCursorPosition(double x, double y);
    void handleMouseButton(int button, int action);
    void handleKey(int key, int action);
    void update(float deltaTime);
    void updateCombat(float deltaTime);
    void render();
    void spawnEnemy(EnemyType type);
    void defeatEnemy(Enemy& enemy);
    void resetEnemies();
    void resetPlayers();
    void restartGame();
    void purchaseAndPlacePlayer(int card, int lane);
    bool playerCardIsUnlocked(int card) const;
    int playerCostForCard(int card) const;
    static glm::vec2 cursorToNdc(double x, double y);
    static int laneFromY(float y);

    GLFWwindow* window_ = nullptr;
    Renderer renderer_;
    UI ui_;
    Menu menu_;
    WaveManager waveManager_;
    std::vector<Player> players_;
    std::vector<Enemy> enemies_;
    std::vector<Projectile> enemyProjectiles_;
    std::vector<Projectile> playerProjectiles_;
    std::vector<Collectible> collectibles_;
    bool selectedLanes_[3] = {false, false, false};
    int hoveredLane_ = -1;
    bool duckKeyHeld_ = false;
    int coins_ = 30;
    int coinsEarnedSinceStart_ = 0;
    int placementLane_ = -1;
    int hoveredPlayerCard_ = -1;
    bool paused_ = false;
    bool gameOver_ = false;
    bool gameWon_ = false;
    float lastFrameTime_ = 0.0f;
};
