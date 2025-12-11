#include "EnemyManager.h"



void EnemyManager::Clear() {
	enemies.clear(); // unique_ptr が自動削除
}

void EnemyManager::Initialize(Camera* camera) {
	Clear();

	const int enemyCount = 100;

	// 敵を配置する全体の X 範囲
	float startX = 10.0f; // 左側の開始位置
	float endX = 110.0f;  // 右側の終了位置

	for (int i = 0; i < enemyCount; i++) {

		// 進行度 (0.0 ～ 1.0)
		float t = (float)i / (enemyCount - 1);

		// ----------------------------
		// ★ X位置：前半は広く、後半は狭く詰まる
		// ----------------------------
		float spacingStart = 5.0f; // 左側（密度低い）
		float spacingEnd = 1.0f;   // 右側（密度高い）

		// 線形補間（左→右でだんだん間隔が狭くなる）
		float spacing = spacingStart + (spacingEnd - spacingStart) * t;

		// 位置計算
		float x = startX + i * spacing;

		// ----------------------------
		// ★ Y位置：前半は固定、後半はランダム
		// ----------------------------
		float y = 2.0f; // 初期高さ

		if (t > 0.5f) {
			// 後半はランダム（1.5 ～ 5.0）
			y = 1.5f + ((float)rand() / RAND_MAX) * (5.0f - 1.5f);
		}

		Vector3 pos = {x, y, 0.0f};

		AddEnemy(camera, pos);
	}
}

void EnemyManager::AddEnemy(Camera* camera, const Vector3& pos) {
	auto e = std::make_unique<Enemy>();
	e->Initialize(camera, pos);
	e->SetCamera(camera);
	enemies.push_back(std::move(e));
}

void EnemyManager::Update(Camera* camera) {
	for (auto& e : enemies) { // ← unique_ptr 参照に変更
		if (e->GetIsAlive()) {
			e->SetCamera(camera);
			e->Update();
		}
	}
}

void EnemyManager::Draw() {
	for (auto& e : enemies) {
		if (e->GetIsAlive()) {
			e->Draw();
		}
	}
}