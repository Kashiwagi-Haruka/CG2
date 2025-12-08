#pragma once
#include "Object3d.h"
#include "Transform.h"

class Camera;
class BulletLine {

private:
	Object3d* lineObject_ = nullptr;
	Transform lineTransform_;
	Camera* camera_ = nullptr;

	Vector3 playerPos_{0, 0, 0};
	Vector3 shotDir_{1, 0, 0}; // デフォルトは右向き
	float lineLength_ = 5.0f;  // 予測線の長さ

public:
	BulletLine();
	~BulletLine();

	void Initialize();
	void Update();
	void Draw();

	void SetCamera(Camera* camera) { camera_ = camera; }
	void SetPlayerPos(const Vector3& pos) { playerPos_ = pos; }
	void SetShotDir(const Vector3& dir) { shotDir_ = dir; }
};
