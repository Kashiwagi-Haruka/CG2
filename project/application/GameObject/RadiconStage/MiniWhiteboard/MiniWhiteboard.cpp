#include "MiniWhiteboard.h"
#include "Function.h"
#include <algorithm>

bool MiniWhiteboard::isSendTooSmallMessage_ = false;

void MiniWhiteboard::Initialize() {
	transform_.scale = {0.25f, 0.25f, 0.25f};
	transform_.rotate = {0.0f, 3.14159265f, 0.0f};
	transform_.translate = {2.8f, 0.0f, 2.0f};
	wasPlayerTouchingPortal_ = false;
	isSendTooSmallMessage_ = false;
}

void MiniWhiteboard::Update(const Vector3& playerPosition, float playerRadius) {
	const bool isTouchingPortal = IsPlayerTouchingPortal(playerPosition, playerRadius);
	if (isTouchingPortal && !wasPlayerTouchingPortal_) {
		isSendTooSmallMessage_ = true;
	}
	wasPlayerTouchingPortal_ = isTouchingPortal;
}

AABB MiniWhiteboard::GetPortalAABB() const {
	const Vector3 center = transform_.translate + portalLocalOffset_;
	return {
	    .min = {center.x - portalHalfSize_.x, center.y - portalHalfSize_.y, center.z - portalHalfSize_.z},
	    .max = {center.x + portalHalfSize_.x, center.y + portalHalfSize_.y, center.z + portalHalfSize_.z},
	};
}

bool MiniWhiteboard::ConsumeTooSmallMessage() {
	const bool result = isSendTooSmallMessage_;
	isSendTooSmallMessage_ = false;
	return result;
}

bool MiniWhiteboard::IsPlayerTouchingPortal(const Vector3& playerPosition, float playerRadius) const {
	const AABB portalAABB = GetPortalAABB();
	const float nearestX = std::clamp(playerPosition.x, portalAABB.min.x, portalAABB.max.x);
	const float nearestY = std::clamp(playerPosition.y, portalAABB.min.y, portalAABB.max.y);
	const float nearestZ = std::clamp(playerPosition.z, portalAABB.min.z, portalAABB.max.z);

	const float dx = playerPosition.x - nearestX;
	const float dy = playerPosition.y - nearestY;
	const float dz = playerPosition.z - nearestZ;
	return (dx * dx + dy * dy + dz * dz) <= (playerRadius * playerRadius);
}