#pragma once

#include "Enemy.h"

#include <cstdlib>

// Header-only so the existing CMake target can stay unchanged.
class WaveManager {
public:
    explicit WaveManager(float spawnInterval = 3.0f)
        : spawnInterval_(spawnInterval) {}

    void reset() {
        waveIndex_ = 0;
        spawnedInWave_ = 0;
        spawnTimer_ = 0.0f;
        firstSpawnPending_ = true;
        complete_ = false;
    }

    // Emits no more than one enemy per update. A new wave starts only after
    // every enemy from the previous wave has been defeated.
    bool update(float deltaTime, int aliveEnemyCount, EnemyType& spawnedType) {
        if (complete_) return false;

        if (spawnedInWave_ == waveSize(waveIndex_)) {
            if (aliveEnemyCount > 0) return false;
            ++waveIndex_;
            spawnedInWave_ = 0;
            spawnTimer_ = 0.0f;
            firstSpawnPending_ = true;
            if (waveIndex_ >= 3) {
                complete_ = true;
                return false;
            }
        }

        if (!firstSpawnPending_) {
            spawnTimer_ += deltaTime;
            if (spawnTimer_ < spawnInterval_) return false;
            spawnTimer_ -= spawnInterval_;
        } else {
            firstSpawnPending_ = false;
        }

        spawnedType = chooseEnemyType(waveIndex_, spawnedInWave_);
        ++spawnedInWave_;
        return true;
    }

    bool isComplete() const { return complete_; }
    int currentWaveNumber() const { return complete_ ? 3 : waveIndex_ + 1; }
    int currentWaveSize() const { return complete_ ? 0 : waveSize(waveIndex_); }
    int spawnedInCurrentWave() const { return spawnedInWave_; }

private:
    static int waveSize(int waveIndex) {
        static constexpr int sizes[] = {4, 8, 12};
        return sizes[waveIndex < 0 ? 0 : (waveIndex > 2 ? 2 : waveIndex)];
    }

    static EnemyType chooseEnemyType(int waveIndex, int enemyIndex) {
        if (waveIndex == 0) return EnemyType::EasyUpperRanged;
        if (waveIndex == 1) {
            if (enemyIndex == 0) return EnemyType::EasyUpperRanged;
            if (enemyIndex == 1) return EnemyType::MediumLowerRanged;
            return (std::rand() % 2 == 0)
                ? EnemyType::EasyUpperRanged : EnemyType::MediumLowerRanged;
        }
        if (enemyIndex == 0) return EnemyType::EasyUpperRanged;
        if (enemyIndex == 1) return EnemyType::MediumLowerRanged;
        if (enemyIndex == 2) return EnemyType::HardMelee;
        return static_cast<EnemyType>(std::rand() % 3);
    }

    float spawnInterval_;
    float spawnTimer_ = 0.0f;
    int waveIndex_ = 0;
    int spawnedInWave_ = 0;
    bool firstSpawnPending_ = true;
    bool complete_ = false;
};
