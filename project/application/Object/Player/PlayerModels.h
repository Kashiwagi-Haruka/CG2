#pragma once
#include "Animation/Animation.h"
#include "Animation/Skeleton.h"
#include "Animation/AnimationManager.h"
#include "Animation/SkinCluster.h"
#include "Camera.h"
#include "Object3d/Object3d.h"
#include "Transform.h"
#include <memory>
#include <optional>
#include <vector>
#include "Function.h"
class PlayerModels {

public:
	enum StateM {
		idle,
		walk,
		attack1,
		attack2,
		attack3,
		attack4,
		fallingAttack,
		skillAttack,
		damage,

	};

private:
	StateM state_;
	Matrix4x4 playerWorld;
	std::unique_ptr<Object3d> Sizuku_;
	Transform player_;
	Camera* camera_;
	std::unique_ptr<Skeleton> sizukuSkeleton_{};
	SkinCluster sizukuSkinCluster_{};
	Animation::AnimationData blendedPoseAnimation_{};
	const std::string animationGroupName_ = "sizuku";
	const float kAnimationBlendDuration_ = 0.3f;
	bool animationFinished_ = false;
	

public:
	PlayerModels();
	~PlayerModels();
	void SetCamera(Camera* camera) { camera_ = camera; };
	void SetPlayerTransform(Transform player) { player_ = player; };
	void SetStateM(StateM state) { state_ = state; }
	void Initialize();
	void Update();
	void Draw();
	std::optional<Matrix4x4> GetJointWorldMatrix(const std::string& jointName) const;
	bool IsAttackAnimationFinished() const { return animationFinished_; }
};