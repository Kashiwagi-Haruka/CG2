#pragma once
#include "Enemy.h"
#include <memory>
#include <vector>

class EnemyManager {

public:
	std::vector<std::unique_ptr<Enemy>> enemies;

	EnemyManager() {}
	~EnemyManager() = default;

	void Initialize(Camera* camera);
	void AddEnemy(Camera* camera, const Vector3& pos);
	void Update(Camera* camera);
	void Draw();
	void Clear();
};
