#pragma once
#include "Enemy.h"
#include <vector>

class EnemyManager {

public:
	std::vector<Enemy*> enemies;

	EnemyManager() {}
	~EnemyManager();

	void Initialize(Camera* camera);

	// 敵追加
	void AddEnemy(Camera* camera, const Vector3& pos);

	// 更新
	void Update(Camera* camera);

	// 描画
	void Draw();

	// 全削除
	void Clear();
};
