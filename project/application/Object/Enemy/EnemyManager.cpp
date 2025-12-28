#include "EnemyManager.h"
#include <cstdlib>

void EnemyManager::Clear() {
	enemies.clear(); // unique_ptr が自動削除
}

void EnemyManager::Initialize(Camera* camera) {
	Clear();
	camera_ = camera;

	// ウェーブシステムの初期化
	currentWave_ = 0;
	waveState_ = WaveState::kWaiting;
	waveTimer_ = 0.0f;
	waveDelay_ = 3.0f; // ウェーブ間の待機時間（秒）
	totalEnemiesKilled_ = 0;

	// 最初のウェーブを開始
	StartNextWave();
}

void EnemyManager::StartNextWave() {
	currentWave_++;
	waveState_ = WaveState::kSpawning;
	waveTimer_ = 0.0f;

	// 現在のウェーブに基づいて敵を生成
	SpawnWaveEnemies();
}

void EnemyManager::SpawnWaveEnemies() {

	// ウェーブごとの設定
	struct WaveConfig {
		int enemyCount;    // 敵の数
		float startX;      // 開始X位置
		float endX;        // 終了X位置
		float minY;        // 最小Y位置
		float maxY;        // 最大Y位置
		bool randomHeight; // ランダムな高さにするか
	};

	WaveConfig config;

	switch (currentWave_) {
	case 1: // ウェーブ1: 少数、低い位置、広い間隔
		config = {5, -40.0f, 30.0f, 1.5f, 2.0f, false};
		break;

	case 2: // ウェーブ2: 中数、やや高い位置
		config = {8, -40.0f, 40.0f, 1.5f, 3.0f, true};
		break;

	case 3: // ウェーブ3: 多数、バラバラの高さ
		config = {12, -40.0f, 50.0f, 1.5f, 4.0f, true};
		break;

	case 4: // ウェーブ4: 密集、高低差大
		config = {15, -40.0f, 45.0f, 1.0f, 5.0f, true};
		break;

	case 5: // ウェーブ5: 大量、ランダム配置
		config = {20, -40.0f, 60.0f, 1.0f, 6.0f, true};
		break;

	default: // ウェーブ6以降: どんどん増える
		config = {
		    15 + (currentWave_ - 5) * 3, // 徐々に増加
		    10.0f,
		    50.0f + (currentWave_ - 5) * 5.0f,
		    1.0f,
		    6.0f,
		    true};
		break;
	}

	// 敵を生成
	float spacing = (config.endX - config.startX) / config.enemyCount;

	for (int i = 0; i < config.enemyCount; i++) {
		float x = config.startX + i * spacing;

		float y;
		if (config.randomHeight) {
			// ランダムな高さ
			y = config.minY + ((float)rand() / RAND_MAX) * (config.maxY - config.minY);
		} else {
			// 固定の高さ
			y = config.minY;
		}

		// ランダムなZ位置のバリエーション
		float z = -52.0f + ((float)rand() / RAND_MAX) * 4.0f;

		Vector3 pos = {x, y, z};
		AddEnemy(camera_, pos);
	}

	waveState_ = WaveState::kActive;
}

void EnemyManager::AddEnemy(Camera* camera, const Vector3& pos) {
	auto e = std::make_unique<Enemy>();
	e->Initialize(camera, pos);
	e->SetCamera(camera);
	enemies.push_back(std::move(e));
}

void EnemyManager::Update(Camera* camera) {

	// ウェーブの状態管理
	switch (waveState_) {
	case WaveState::kWaiting:
		// 待機中（次のウェーブまでの猶予時間）
		waveTimer_ += 1.0f / 60.0f;
		if (waveTimer_ >= waveDelay_) {
			StartNextWave();
		}
		break;

	case WaveState::kActive:
		// ウェーブ進行中：全滅チェック
		CheckWaveComplete();
		break;

	case WaveState::kComplete:
		// ウェーブクリア：次のウェーブへ
		waveState_ = WaveState::kWaiting;
		waveTimer_ = 0.0f;
		break;

	default:
		break;
	}

	// 敵の更新
	for (auto& e : enemies) {
		if (e->GetIsAlive()) {
			e->SetCamera(camera);
			e->Update();
		}
	}
}

void EnemyManager::CheckWaveComplete() {
	// 生存している敵の数をカウント
	int aliveCount = 0;
	for (auto& e : enemies) {
		if (e->GetIsAlive()) {
			aliveCount++;
		}
	}

	// 全滅したらウェーブクリア
	if (aliveCount == 0 && !enemies.empty()) {
		waveState_ = WaveState::kComplete;
		totalEnemiesKilled_ += static_cast<int>(enemies.size());

		// 倒した敵をクリア
		enemies.clear();
	}
}

void EnemyManager::Draw() {
	for (auto& e : enemies) {
		if (e->GetIsAlive()) {
			e->Draw();
		}
	}
}

int EnemyManager::GetAliveEnemyCount() const {
	int count = 0;
	for (const auto& e : enemies) {
		if (e->GetIsAlive()) {
			count++;
		}
	}
	return count;
}

bool EnemyManager::IsWaveActive() const { return waveState_ == WaveState::kActive; }

bool EnemyManager::IsWaveComplete() const { return waveState_ == WaveState::kComplete; }

float EnemyManager::GetWaveProgress() const {
	if (enemies.empty())
		return 1.0f;

	int aliveCount = GetAliveEnemyCount();
	int totalCount = static_cast<int>(enemies.size());

	return 1.0f - ((float)aliveCount / totalCount);
}