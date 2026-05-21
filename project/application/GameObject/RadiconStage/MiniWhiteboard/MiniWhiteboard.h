#pragma once

#include "RigidBody.h"
#include "Transform.h"
#include "Vector3.h"
#include "Object3d/Object3d.h"

class MiniWhiteboard {
public:
	void Initialize();
	void Update(const Vector3& playerPosition, float playerRadius = 0.35f);

	const Transform& GetTransform() const { return transform_; }
	AABB GetPortalAABB() const;

	static bool ConsumeTooSmallMessage();

private:
	bool IsPlayerTouchingPortal(const Vector3& playerPosition, float playerRadius) const;

private:
	std::unique_ptr<Object3d> obj_;
	Transform transform_{};
	Vector3 portalLocalOffset_{0.0f, 0.9f, 0.0f};
	Vector3 portalHalfSize_{0.22f, 0.35f, 0.08f};
	bool wasPlayerTouchingPortal_ = false;
	static bool isSendTooSmallMessage_;
};