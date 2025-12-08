#pragma once
#include "BulletLine.h"

#include "PlayerBullet.h"
#include <vector>
#include <memory>

class Camera;

class BulletManager {

	Camera* camera_ = nullptr;

std::vector<std::unique_ptr<PlayerBullet>> bullets_;

	BulletLine line_;                   // ★ 予測線は1つ

public:
	void Initialize(Camera* camera);
	void Update();
	void Draw();

	void SetPlayerPos(const Vector3& pos); // 予測線更新用
	void SetShotDir(const Vector3& dir);   // 入力方向
	bool Collision(Vector3 position);
	void Fire(const Vector3& pos, const Vector3& dir);
	void SetCamera(Camera* camera);
	const std::vector<std::unique_ptr<PlayerBullet>>& GetBullets() const { return bullets_; }


};
