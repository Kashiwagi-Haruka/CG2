#pragma once
#include "Camera.h"
#include "Matrix4x4.h"
#include "Primitive/Primitive.h"
#include "Vector3.h"
#include <deque>
#include <memory>
#include <vector>

class PlayerSwordTrail {
public:
	void Initialize();
	void Update(const Matrix4x4& swordWorldMatrix, bool isAttacking);
	void Draw();
	void Clear();

	void SetCamera(Camera* camera) { camera_ = camera; }

private:
	void EnsureSegmentCount(size_t count);
	void UpdateSegments();

	static constexpr float kTipLocalOffset = 1.2f;
	static constexpr float kSegmentWidth = 0.25f;
	static constexpr float kMinPointDistance = 0.05f;
	static constexpr size_t kMaxPoints = 14;

	std::deque<Vector3> points_;
	std::vector<std::unique_ptr<Primitive>> segments_;
	Camera* camera_ = nullptr;
};