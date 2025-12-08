#include "EnemyManager.h"

EnemyManager::~EnemyManager() { Clear(); }

void EnemyManager::Clear() {
	for (auto e : enemies) {
		delete e;
	}
	enemies.clear();
}

void EnemyManager::Initialize(Camera* camera) {
	Clear();

	// ここで最初から15体出す例
	for (int i = 0; i < 15; i++) {
		Vector3 pos = {float(i * 1)+10, 2.0f, 0.0f};
		AddEnemy(camera, pos);
	}
}

void EnemyManager::AddEnemy(Camera* camera, const Vector3& pos) {
	Enemy* e = new Enemy();
	e->Initialize(camera,pos);
	e->SetCamera(camera); // 念のため
	
	enemies.push_back(e);
}

void EnemyManager::Update(Camera* camera) {
	for (auto e : enemies) {
		if (e->GetIsAlive()) {

			e->SetCamera(camera);
			e->Update();
		}
	}
}

void EnemyManager::Draw() {
	for (auto e : enemies) {
		if (e->GetIsAlive()) {
			e->Draw();
		}
	}
}
