#include "WaveManager.h"

#include <cstdlib>

WaveManager::WaveManager(float spawnInterval) : spawnInterval_(spawnInterval) {}

void WaveManager::reset() {
    waveIndex_ = 0;
    spawnedInWave_ = 0;
    spawnTimer_ = 0.0f;
    firstSpawnPending_ = true;
    complete_ = false;
}

bool WaveManager::update(float deltaTime, int aliveEnemyCount, EnemyType& spawnedType) {
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

bool WaveManager::isComplete() const {
    return complete_;
}
int WaveManager::currentWaveNumber() const {
    if (complete_) return 3;
    return waveIndex_ + 1;
}
int WaveManager::currentWaveSize() const {
    if (complete_) return 0;
    return waveSize(waveIndex_);
}
int WaveManager::spawnedInCurrentWave() const {
    return spawnedInWave_;
}

int WaveManager::waveSize(int waveIndex) {
    static constexpr int sizes[] = {3, 5, 10};
    int safeIndex = waveIndex;
    if (safeIndex < 0) safeIndex = 0;
    if (safeIndex > 2) safeIndex = 2;
    return sizes[safeIndex];
}

EnemyType WaveManager::chooseEnemyType(int waveIndex, int enemyIndex) {
    if (waveIndex == 0) return EnemyType::EasyUpperRanged;
    if (waveIndex == 1) {
        if (enemyIndex == 0) return EnemyType::EasyUpperRanged;
        if (enemyIndex == 1) return EnemyType::MediumLowerRanged;
        if (std::rand() % 2 == 0) return EnemyType::EasyUpperRanged;
        return EnemyType::MediumLowerRanged;
    }
    if (enemyIndex == 0) return EnemyType::EasyUpperRanged;
    if (enemyIndex == 1) return EnemyType::MediumLowerRanged;
    if (enemyIndex == 2) return EnemyType::HardMelee;
    return static_cast<EnemyType>(std::rand() % 3);
}
