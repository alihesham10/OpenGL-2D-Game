#include "Game.h"

#include "GameConfig.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <iostream>

namespace {
float contactDistance(const Player& player, const Enemy& enemy) {
    float distance = 0.5f * (player.bodyWidth() + enemy.bodyWidth());
    if (player.type == PlayerType::Melee) distance += GameConfig::MELEE_WEAPON_REACH_NDC;
    return distance;
}

float smoothProgress(float value) {
    const float t = std::clamp(value, 0.0f, 1.0f);
    return t * t * (3.0f - 2.0f * t);
}
}

Game::Game() : waveManager_(GameConfig::ENEMY_SPAWN_INTERVAL) {}

void Game::cursorPositionCallback(GLFWwindow* window, double x, double y) {
    Game* game = static_cast<Game*>(glfwGetWindowUserPointer(window));
    if (game) game->handleCursorPosition(x, y);
}

void Game::mouseButtonCallback(GLFWwindow* window, int button, int action, int) {
    Game* game = static_cast<Game*>(glfwGetWindowUserPointer(window));
    if (game) game->handleMouseButton(button, action);
}

void Game::keyCallback(GLFWwindow* window, int key, int, int action, int) {
    Game* game = static_cast<Game*>(glfwGetWindowUserPointer(window));
    if (game) game->handleKey(key, action);
}

glm::vec2 Game::cursorToNdc(double x, double y) {
    return glm::vec2(static_cast<float>(2.0 * x / GameConfig::WINDOW_W - 1.0),
                     static_cast<float>(1.0 - 2.0 * y / GameConfig::WINDOW_H));
}

int Game::laneFromY(float y) {
    for (int i = 0; i < GameConfig::LANE_COUNT; ++i) {
        const float laneTop = GameConfig::LANE_Y[i] + GameConfig::LANE_HEIGHT * 0.5f;
        const float laneBottom = GameConfig::LANE_Y[i] - GameConfig::LANE_HEIGHT * 0.5f;
        if (y >= laneBottom && y <= laneTop) return i;
    }
    return -1;
}

bool Game::playerCardIsUnlocked(int card) const {
    return Player::cardIsUnlocked(card, coins_, coinsEarnedSinceStart_);
}

int Game::playerCostForCard(int card) const {
    return Player::costForCard(card);
}

void Game::handleCursorPosition(double x, double y) {
    const glm::vec2 position = cursorToNdc(x, y);
    if (menu_.state() == MenuState::Cutscene) return;
    if (menu_.state() == MenuState::MainMenu) {
        menu_.setHoveredMainOption(menu_.mainMenuOptionAt(position.x, position.y));
        menu_.setHoveredPauseOption(-1);
        hoveredLane_ = -1;
        hoveredPlayerCard_ = -1;
        return;
    }
    if (menu_.state() == MenuState::Paused) {
        menu_.setHoveredPauseOption(menu_.pauseMenuOptionAt(position.x, position.y));
        menu_.setHoveredMainOption(-1);
        hoveredLane_ = -1;
        hoveredPlayerCard_ = -1;
        return;
    }
    if (menu_.state() == MenuState::GameOver || menu_.state() == MenuState::GameWon) {
        menu_.setHoveredResultOption(menu_.resultMenuOptionAt(position.x, position.y));
        menu_.setHoveredMainOption(-1);
        menu_.setHoveredPauseOption(-1);
        hoveredLane_ = -1;
        hoveredPlayerCard_ = -1;
        return;
    }
    hoveredLane_ = laneFromY(position.y);
    hoveredPlayerCard_ = -1;
    for (int card = 0; card < 3; ++card) {
        if (std::abs(position.x - GameConfig::PLAYER_CARD_X[card]) <= GameConfig::PLAYER_CARD_HALF_WIDTH &&
            std::abs(position.y - GameConfig::PLAYER_CARD_Y) <= GameConfig::PLAYER_CARD_HALF_HEIGHT) {
            if ((card == 0 && coins_ >= GameConfig::MELEE_PLAYER_COST) ||
                (card == 1 && coinsEarnedSinceStart_ >= GameConfig::LONG_RANGE_UNLOCK_THRESHOLD &&
                 coins_ >= GameConfig::LONG_RANGE_PLAYER_COST) ||
                (card == 2 && coinsEarnedSinceStart_ >= GameConfig::DEFENSE_UNLOCK_THRESHOLD &&
                 coins_ >= GameConfig::DEFENSE_PLAYER_COST)) {
                hoveredPlayerCard_ = card;
            }
            break;
        }
    }
}

void Game::handleMouseButton(int button, int action) {
    if (menu_.state() == MenuState::Cutscene) return;
    if (menu_.state() == MenuState::MainMenu) {
        if (button != GLFW_MOUSE_BUTTON_LEFT || action != GLFW_PRESS) return;
        double x, y;
        glfwGetCursorPos(window_, &x, &y);
        const glm::vec2 position = cursorToNdc(x, y);
        const int option = menu_.mainMenuOptionAt(position.x, position.y);
        if (option == 0) {
            startIntroCutscene();
        } else if (option == 1) {
            glfwSetWindowShouldClose(window_, GLFW_TRUE);
        }
        return;
    }
    if (menu_.state() == MenuState::Paused) {
        if (button != GLFW_MOUSE_BUTTON_LEFT || action != GLFW_PRESS) return;
        double x, y;
        glfwGetCursorPos(window_, &x, &y);
        const glm::vec2 position = cursorToNdc(x, y);
        const int option = menu_.pauseMenuOptionAt(position.x, position.y);
        if (option == 0) {
            paused_ = false;
            menu_.clear();
        } else if (option == 1) {
            restartGame();
        } else if (option == 2) {
            restartGame();
            menu_.showMainMenu();
        }
        return;
    }
    if (menu_.state() == MenuState::GameOver || menu_.state() == MenuState::GameWon) {
        if (button != GLFW_MOUSE_BUTTON_LEFT || action != GLFW_PRESS) return;
        double x, y;
        glfwGetCursorPos(window_, &x, &y);
        const glm::vec2 position = cursorToNdc(x, y);
        const int option = menu_.resultMenuOptionAt(position.x, position.y);
        if (option == 0) {
            restartGame();
        } else if (option == 1) {
            restartGame();
            menu_.showMainMenu();
        }
        return;
    }
    if (gameOver_ || gameWon_) return;
    if (button != GLFW_MOUSE_BUTTON_LEFT || action != GLFW_PRESS) return;
    double x, y;
    glfwGetCursorPos(window_, &x, &y);
    const glm::vec2 position = cursorToNdc(x, y);
    for (int card = 0; card < 3; ++card) {
        if (std::abs(position.x - GameConfig::PLAYER_CARD_X[card]) <= GameConfig::PLAYER_CARD_HALF_WIDTH &&
            std::abs(position.y - GameConfig::PLAYER_CARD_Y) <= GameConfig::PLAYER_CARD_HALF_HEIGHT) {
            if (playerCardIsUnlocked(card) && placementLane_ >= 0 && selectedLanes_[placementLane_])
                purchaseAndPlacePlayer(card, placementLane_);
            return;
        }
    }
    const int lane = laneFromY(position.y);
    if (lane >= 0) {
        selectedLanes_[lane] = !selectedLanes_[lane];
        if (selectedLanes_[lane]) {
            placementLane_ = lane;
        } else if (placementLane_ == lane) {
            placementLane_ = -1;
            for (int i = 0; i < GameConfig::LANE_COUNT; ++i) {
                if (selectedLanes_[i]) placementLane_ = i;
            }
        }
    }
}

void Game::handleKey(int key, int action) {
    if (menu_.state() == MenuState::Cutscene) return;
    if (menu_.state() == MenuState::MainMenu) {
        if (action == GLFW_PRESS && (key == GLFW_KEY_ENTER || key == GLFW_KEY_SPACE))
            startIntroCutscene();
        else if (action == GLFW_PRESS && key == GLFW_KEY_ESCAPE)
            glfwSetWindowShouldClose(window_, GLFW_TRUE);
        return;
    }
    if (paused_) {
        if (action == GLFW_PRESS && key == GLFW_KEY_ESCAPE) {
            paused_ = false;
            menu_.clear();
        } else if (action == GLFW_PRESS && key == GLFW_KEY_R) {
            restartGame();
        }
        return;
    }
    if (gameOver_ || gameWon_) {
        if (action == GLFW_PRESS && (key == GLFW_KEY_SPACE || key == GLFW_KEY_R))
            restartGame();
        else if (action == GLFW_PRESS && key == GLFW_KEY_M) {
            restartGame();
            menu_.showMainMenu();
        }
        return;
    }
    if (action != GLFW_PRESS && action != GLFW_REPEAT) {
        if ((key == GLFW_KEY_S || key == GLFW_KEY_DOWN) && action == GLFW_RELEASE)
            duckKeyHeld_ = false;
        return;
    }
    if (key == GLFW_KEY_R && action == GLFW_PRESS) {
        restartGame();
        return;
    }
    if (key == GLFW_KEY_S || key == GLFW_KEY_DOWN) {
        duckKeyHeld_ = true;
        return;
    }
    if (action != GLFW_PRESS) return;
    if (key == GLFW_KEY_ESCAPE) {
        paused_ = !paused_;
        menu_.togglePause();
    }
    if (key == GLFW_KEY_UP || key == GLFW_KEY_W) {
        for (Player& player : players_) {
            if (player.alive && player.type != PlayerType::Defense && selectedLanes_[player.lane]) {
                player.jumpTimer = GameConfig::JUMP_DURATION;
                player.ducking = false;
            }
        }
    }
}

void Game::purchaseAndPlacePlayer(int card, int lane) {
    if (!playerCardIsUnlocked(card) || lane < 0 || lane >= GameConfig::LANE_COUNT) return;
    const int cost = playerCostForCard(card);
    if (coins_ < cost) return;
    const PlayerType type = Player::typeForCard(card);
    const float startX = type == PlayerType::Defense
        ? GameConfig::PLAYER_START_X + GameConfig::DEFENSE_START_X_OFFSET_NDC
        : GameConfig::PLAYER_START_X;
    players_.emplace_back(type, glm::vec2(startX, GameConfig::LANE_Y[lane]), lane);
    coins_ -= cost;
}

void Game::spawnEnemy(EnemyType type) {
    const int lane = std::rand() % GameConfig::LANE_COUNT;
    enemies_.emplace_back(type, glm::vec2(0.90f, GameConfig::LANE_Y[lane]), lane);
}

void Game::defeatEnemy(Enemy& enemy) {
    if (!enemy.alive) return;
    enemy.alive = false;
    enemy.engaged = false;
    collectibles_.emplace_back(enemy.pos, enemy.lane, GameConfig::ENEMY_COIN_REWARD);
}

void Game::resetEnemies() {
    enemies_.clear();
    enemyProjectiles_.clear();
    playerProjectiles_.clear();
    collectibles_.clear();
    waveManager_.reset();
}

void Game::resetPlayers() {
    players_.clear();
    selectedLanes_[0] = false;
    selectedLanes_[1] = false;
    selectedLanes_[2] = false;
    placementLane_ = -1;
    coinsEarnedSinceStart_ = 0;
    duckKeyHeld_ = false;
}

void Game::restartGame() {
    resetPlayers();
    resetEnemies();
    coins_ = GameConfig::STARTING_COINS;
    gameOver_ = false;
    gameWon_ = false;
    paused_ = false;
    cutsceneHeroes_.clear();
    cutsceneEnemies_.clear();
    cutsceneElapsed_ = 0.0f;
    menu_.clear();
}

void Game::startIntroCutscene() {
    restartGame();
    cutsceneHeroes_.emplace_back(PlayerType::Melee, glm::vec2(-0.52f, -0.32f), 1);
    cutsceneHeroes_.emplace_back(PlayerType::LongRange, glm::vec2(-0.25f, -0.32f), 1);
    cutsceneHeroes_.emplace_back(PlayerType::Defense, glm::vec2(0.02f, -0.32f), 1);

    cutsceneEnemies_.emplace_back(EnemyType::HardMelee, glm::vec2(1.18f, -0.32f), 1);
    cutsceneEnemies_.emplace_back(EnemyType::MediumLowerRanged, glm::vec2(1.40f, -0.32f), 1);
    cutsceneEnemies_.emplace_back(EnemyType::EasyUpperRanged, glm::vec2(1.62f, -0.32f), 1);
    for (Enemy& enemy : cutsceneEnemies_) enemy.alive = false;

    menu_.showCutscene();
}

void Game::update(float deltaTime) {
    if (menu_.state() == MenuState::Cutscene) {
        updateCutscene(deltaTime);
        return;
    }
    if (menu_.state() != MenuState::MainMenu && !paused_ && !gameOver_ && !gameWon_)
        updateCombat(deltaTime);
}

void Game::updateCutscene(float deltaTime) {
    cutsceneElapsed_ += deltaTime;
    if (cutsceneElapsed_ >= 10.0f) {
        restartGame();
        return;
    }

    const float time = cutsceneElapsed_;
    const float idleBob = 0.008f * std::sin(time * 4.0f);
    const float heroStep = smoothProgress((time - 5.0f) / 1.3f);
    const float alarm = smoothProgress((time - 2.0f) / 0.25f) *
        (1.0f - smoothProgress((time - 2.25f) / 0.25f));
    cutsceneHeroes_[0].pos = glm::vec2(-0.52f - 0.018f * alarm + 0.13f * heroStep,
                                      -0.32f + idleBob);
    cutsceneHeroes_[1].pos = glm::vec2(-0.25f - 0.014f * alarm + 0.01f * std::sin(time * 3.0f),
                                      -0.32f + 0.006f * std::sin(time * 4.0f + 1.4f));
    cutsceneHeroes_[2].pos = glm::vec2(0.02f - 0.012f * alarm + 0.08f * heroStep,
                                      -0.32f + 0.006f * std::sin(time * 4.0f + 2.8f));

    const float entrance = smoothProgress((time - 2.0f) / 3.0f);
    const float bossX = 1.18f - 0.58f * entrance;
    cutsceneEnemies_[0].alive = time >= 2.0f;
    cutsceneEnemies_[0].pos = glm::vec2(bossX, -0.32f + 0.005f * std::sin(time * 5.0f));
    cutsceneEnemies_[1].alive = time >= 2.35f;
    cutsceneEnemies_[1].pos = glm::vec2(bossX + 0.20f,
                                        -0.32f + 0.005f * std::sin(time * 5.0f + 1.0f));
    cutsceneEnemies_[2].alive = time >= 2.70f;
    cutsceneEnemies_[2].pos = glm::vec2(bossX + 0.40f,
                                        -0.32f + 0.005f * std::sin(time * 5.0f + 2.0f));

    const float preparation = smoothProgress((time - 5.0f) / 1.2f);
    cutsceneHeshamAttacking_ = time >= 5.0f && std::sin(time * 8.0f) > -0.15f;
    cutsceneFahmyAiming_ = time >= 5.0f;
    cutsceneShikoShieldRaise_ = 0.075f * preparation;
    cutsceneBossWeaponRotation_ = -0.25f - 0.85f * preparation +
        (time >= 5.0f ? 0.07f * std::sin(time * 5.0f) : 0.0f);
}

void Game::updateCombat(float deltaTime) {
    int aliveEnemyCount = 0;
    for (const Enemy& enemy : enemies_) if (enemy.alive) ++aliveEnemyCount;
    EnemyType nextEnemyType;
    if (waveManager_.update(deltaTime, aliveEnemyCount, nextEnemyType)) spawnEnemy(nextEnemyType);
    if (waveManager_.isComplete() && aliveEnemyCount == 0) {
        gameWon_ = true;
        menu_.showGameWon();
        return;
    }

    for (Player& player : players_) {
        if (!player.alive) continue;
        player.update(deltaTime,
                      player.lane >= 0 && player.lane < GameConfig::LANE_COUNT && selectedLanes_[player.lane],
                      duckKeyHeld_);
        if (player.engagedEnemy >= 0 &&
            (player.engagedEnemy >= static_cast<int>(enemies_.size()) || !enemies_[player.engagedEnemy].alive)) {
            player.engagedEnemy = -1;
            player.attackCooldown = 0.0f;
        }
        player.move(deltaTime);
    }

    for (Enemy& enemy : enemies_) {
        if (!enemy.alive) continue;
        if (!enemy.engaged) {
            const float nextX = enemy.pos.x - 2.0f * enemy.speedPixelsPerSecond /
                GameConfig::WINDOW_W * deltaTime;
            bool waitingAtCombat = false;
            for (const Player& player : players_) {
                if (!player.alive || player.lane != enemy.lane) continue;
                const float distance = contactDistance(player, enemy);
                if (enemy.pos.x >= player.pos.x && nextX <= player.pos.x + distance) {
                    enemy.pos.x = player.pos.x + distance;
                    waitingAtCombat = true;
                    break;
                }
            }
            if (!waitingAtCombat) enemy.moveLeft(deltaTime);
        }
        if (enemy.pos.x <= GameConfig::HOME_BASE_X) {
            gameOver_ = true;
            menu_.showGameOver();
            return;
        }

        const int defeatedPlayer = enemy.attack(players_, enemyProjectiles_, deltaTime);
        if (defeatedPlayer >= 0) {
            Player& player = players_[defeatedPlayer];
            if (player.engagedEnemy >= 0 && player.engagedEnemy < static_cast<int>(enemies_.size()))
                enemies_[player.engagedEnemy].engaged = false;
            player.engagedEnemy = -1;
        }
    }

    for (Projectile& projectile : enemyProjectiles_) {
        if (!projectile.alive) continue;
        const float previousX = projectile.pos.x;
        projectile.update(deltaTime);
        Player* firstHit = nullptr;
        for (Player& player : players_) {
            if (!player.alive || player.lane != projectile.lane) continue;
            const float collisionDistance = 0.5f * player.bodyWidth();
            const bool crossed = projectile.crossedTarget(previousX,
                player.pos.x - collisionDistance, player.pos.x + collisionDistance);
            if (crossed && !projectile.isAvoidedBy(player) &&
                (!firstHit || player.pos.x > firstHit->pos.x)) firstHit = &player;
        }
        if (firstHit) {
            firstHit->takeDamage(projectile.damage);
            if (!firstHit->alive) {
                if (firstHit->engagedEnemy >= 0 && firstHit->engagedEnemy < static_cast<int>(enemies_.size()))
                    enemies_[firstHit->engagedEnemy].engaged = false;
                firstHit->engagedEnemy = -1;
            }
            projectile.alive = false;
        }
        if (projectile.isOffscreen()) projectile.alive = false;
    }
    enemyProjectiles_.erase(std::remove_if(enemyProjectiles_.begin(), enemyProjectiles_.end(),
        [](const Projectile& projectile) { return !projectile.alive; }), enemyProjectiles_.end());

    for (Player& player : players_) {
        if (!player.alive || player.type == PlayerType::Defense || player.engagedEnemy >= 0) continue;
        int closestEnemy = -1;
        float closestDistance = 100.0f;
        for (int i = 0; i < static_cast<int>(enemies_.size()); ++i) {
            Enemy& enemy = enemies_[i];
            if (!enemy.alive || enemy.engaged || enemy.lane != player.lane) continue;
            const float distance = std::abs(enemy.pos.x - player.pos.x);
            if (distance <= contactDistance(player, enemy) && distance < closestDistance) {
                closestDistance = distance;
                closestEnemy = i;
            }
        }
        if (closestEnemy >= 0) {
            player.engagedEnemy = closestEnemy;
            player.attackCooldown = 0.0f;
            enemies_[closestEnemy].engaged = true;
        }
    }

    for (Player& player : players_) {
        if (!player.alive || player.engagedEnemy < 0) continue;
        Enemy& enemy = enemies_[player.engagedEnemy];
        if (!enemy.alive) {
            player.engagedEnemy = -1;
            continue;
        }
        if (player.attackEnemy(enemy, deltaTime)) {
            defeatEnemy(enemy);
            player.engagedEnemy = -1;
            player.attackCooldown = 0.0f;
        }
    }

    for (Player& player : players_)
        player.fireAtNearestEnemy(enemies_, playerProjectiles_, deltaTime);

    for (Projectile& projectile : playerProjectiles_) {
        if (!projectile.alive) continue;
        const float previousX = projectile.pos.x;
        projectile.update(deltaTime);
        Enemy* firstHit = nullptr;
        for (Enemy& enemy : enemies_) {
            if (!enemy.alive || enemy.lane != projectile.lane) continue;
            const float collisionDistance = 0.5f * enemy.bodyWidth();
            const bool crossed = projectile.crossedTarget(previousX,
                enemy.pos.x - collisionDistance, enemy.pos.x + collisionDistance);
            if (crossed && (!firstHit || enemy.pos.x < firstHit->pos.x)) firstHit = &enemy;
        }
        if (firstHit) {
            firstHit->takeDamage(projectile.damage);
            if (firstHit->health <= 0) {
                defeatEnemy(*firstHit);
                for (Player& player : players_) {
                    if (player.engagedEnemy >= 0 && player.engagedEnemy < static_cast<int>(enemies_.size()) &&
                        &enemies_[player.engagedEnemy] == firstHit) {
                        player.engagedEnemy = -1;
                        player.attackCooldown = 0.0f;
                    }
                }
            }
            projectile.alive = false;
        }
        if (projectile.isOffscreen()) projectile.alive = false;
    }
    playerProjectiles_.erase(std::remove_if(playerProjectiles_.begin(), playerProjectiles_.end(),
        [](const Projectile& projectile) { return !projectile.alive; }), playerProjectiles_.end());

    for (Collectible& collectible : collectibles_) {
        if (!collectible.alive) continue;
        for (const Player& player : players_) {
            if (collectible.collectIfTouched(player)) {
                coins_ += collectible.value;
                coinsEarnedSinceStart_ += collectible.value;
                break;
            }
        }
    }
    collectibles_.erase(std::remove_if(collectibles_.begin(), collectibles_.end(),
        [](const Collectible& collectible) { return !collectible.alive; }), collectibles_.end());
}

void Game::render() {
    if (menu_.state() == MenuState::Cutscene) {
        renderer_.drawCutscene(cutsceneHeroes_, cutsceneEnemies_, cutsceneElapsed_,
                               cutsceneHeshamAttacking_, cutsceneFahmyAiming_,
                               cutsceneShikoShieldRaise_, cutsceneBossWeaponRotation_);
        if (cutsceneElapsed_ < 2.0f) {
            ui_.renderCutsceneDialogue(renderer_, "HESHAM",
                "WE FINALLY FOUND THE TREASURE MAP!");
        } else if (cutsceneElapsed_ < 5.0f) {
            ui_.renderCutsceneDialogue(renderer_, "BOSS",
                "HAND OVER THAT MAP! WE WON'T LEAVE WITHOUT IT!");
        } else if (cutsceneElapsed_ < 6.5f) {
            ui_.renderCutsceneDialogue(renderer_, "FAHMY",
                "YOU'LL HAVE TO GET THROUGH US FIRST!");
        } else if (cutsceneElapsed_ < 8.0f) {
            ui_.renderCutsceneDialogue(renderer_, "SHIKO",
                "THEN LET'S PROTECT IT TOGETHER!");
        } else {
            ui_.renderCutsceneDialogue(renderer_, "", "", "PROTECT THE TREASURE MAP!");
        }
        return;
    }
    renderer_.drawWorld(players_, enemies_, enemyProjectiles_, playerProjectiles_,
                        collectibles_, selectedLanes_, hoveredLane_, gameOver_);
    if (menu_.state() != MenuState::MainMenu)
        ui_.render(renderer_, coins_, coinsEarnedSinceStart_, hoveredPlayerCard_);
    menu_.render(renderer_);
}

int Game::run() {
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW.\n";
        return -1;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    window_ = glfwCreateWindow(GameConfig::WINDOW_W, GameConfig::WINDOW_H,
                               "Temple Defense - Starter", nullptr, nullptr);
    if (!window_) {
        std::cerr << "Could not create GLFW window.\n";
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window_);
    glfwSwapInterval(1);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD.\n";
        glfwDestroyWindow(window_);
        glfwTerminate();
        return -1;
    }
    glViewport(0, 0, GameConfig::WINDOW_W, GameConfig::WINDOW_H);
    glfwSetWindowUserPointer(window_, this);
    glfwSetCursorPosCallback(window_, cursorPositionCallback);
    glfwSetMouseButtonCallback(window_, mouseButtonCallback);
    glfwSetKeyCallback(window_, keyCallback);
    renderer_.initialize();
    std::srand(static_cast<unsigned int>(std::time(nullptr)));
    std::cout << "Controls: click lanes to toggle selection; W/Up jumps and hold S/Down to duck; Esc pauses; R resets.\n";

    int displayedGameState = 0;
    while (!glfwWindowShouldClose(window_)) {
        const int gameState = gameWon_ ? 2 : (gameOver_ ? 1 : 0);
        if (gameState != displayedGameState) {
            if (gameWon_) glfwSetWindowTitle(window_, "Temple Defense - Victory! Press Space to restart");
            else if (gameOver_) glfwSetWindowTitle(window_, "Temple Defense - Game Over. Press Space to restart");
            else glfwSetWindowTitle(window_, "Temple Defense - Starter");
            displayedGameState = gameState;
        }
        const float now = static_cast<float>(glfwGetTime());
        const float deltaTime = std::min(now - lastFrameTime_, 0.05f);
        lastFrameTime_ = now;
        update(deltaTime);
        renderer_.beginFrame();
        render();
        renderer_.present(window_);
        glfwPollEvents();
    }

    renderer_.shutdown();
    glfwDestroyWindow(window_);
    window_ = nullptr;
    glfwTerminate();
    return 0;
}
