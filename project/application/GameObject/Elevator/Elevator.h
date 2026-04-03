#pragma once
#include "Object3d/Object3d.h"
#include "Primitive/Primitive.h"
#include <GameObject/GameCamera/PlayerCamera/PlayerCamera.h>
#include "Animation/Animation.h"
#include "Animation/Skeleton.h"
#include "Animation/SkinCluster.h"
#include <memory>
#include<array>
#include"GameObject/Door/AutoLockSystem.h"
class Camera;

class Elevator{
public:
	Elevator();
	void Initialize();
	void SetCamera(Camera* camera);
	void Update();
	void Draw();
	void SetPlayerCamera(PlayerCamera* camera) { playerCamera_ = camera; };
	std::array<std::unique_ptr<AutoLockSystem>, 2>& GetAutoLockSys() { return autoLockSystems_; };
private:
	// アニメーション
	void Animation();
private:
	Matrix4x4 worldMat_;
	static PlayerCamera* playerCamera_;
	std::unique_ptr<Object3d> modelObj_ = nullptr;
	Transform elevatorTransform_{};
	// アニメーション
	Animation::AnimationData blendedPoseAnimation_{};
	// 骨
	std::unique_ptr<Skeleton> skeleton_{};
	// スキン
	SkinCluster skinCluster_{};
	
	std::string animationGroupName_ = "Elevator";
	const float kAnimationBlendDuration_ = 1.0f;
	bool animationFinished_ = false;
	std::string desiredAnimationName = "Idle";
	static bool isRayHit_;

	float baseHeight_ = 0.0f;

	//オートロック
	std::array<std::unique_ptr<AutoLockSystem>,2>autoLockSystems_;

};