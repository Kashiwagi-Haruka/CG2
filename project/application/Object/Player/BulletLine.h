#pragma once
#include "Object3d.h"
#include "Transform.h"
#include <memory>

class Camera;

class BulletLine {

private:
	std::unique_ptr<Object3d> lineObject_;
	Transform lineTransform_;
	Camera* camera_ = nullptr;

	Vector3 playerPos_{0, 0, 0};
	Vector3 shotDir_{1, 0, 0};
	float lineLength_ = 5.0f;

public:
	BulletLine();
	~BulletLine() = default;

	void Initialize();
	void Update();
	void Draw();

	void SetCamera(Camera* camera) { camera_ = camera; }
	void SetPlayerPos(const Vector3& pos) { playerPos_ = pos; }
	void SetShotDir(const Vector3& dir) { shotDir_ = dir; }
};
