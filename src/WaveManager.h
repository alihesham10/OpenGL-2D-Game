#pragma once
#include "Enemy.h"

class WaveManager {
public:
    explicit WaveManager(float spawnInterval = 3.5f);

    void reset();
    bool update(float deltaTime, int aliveEnemyCount, EnemyType& spawnedType);
    bool isComplete() const;
    int currentWaveNumber() const;
    int currentWaveSize() const;
    int spawnedInCurrentWave() const;

private:
    static int waveSize(int waveIndex);
    static EnemyType chooseEnemyType(int waveIndex, int enemyIndex);

    float spawnInterval_;
    float spawnTimer_ = 0.0f;
    int waveIndex_ = 0;
    int spawnedInWave_ = 0;
    bool firstSpawnPending_ = true;
    bool complete_ = false;
};
