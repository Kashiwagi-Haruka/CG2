#pragma once
#include "Animation/Animation.h"
#include "Animation/Skeleton.h"
#include "Animation/SkinCluster.h"
#include "Transform.h"
#include "Vector3.h"
#include <memory>
#include <vector>

class Camera;
class Object3d;

class Boss {

	int hp_ = 50;
	bool isAlive_ = true;
	float damageInvincibleTimer_ = 0.0f;
	float damageInvincibleDuration_ = 0.5f;
	int lastSkillDamageId_ = -1;

	Transform transform_{};
	Vector3 basePosition_{};
	Vector3 baseScale_{};

	float animationTimer_ = 0.0f;
	float animationTime_ = 0.0f;
	float appearTimer_ = 0.0f;
	float appearDuration_ = 2.0f;

	std::vector<Animation::AnimationData> animationClips_{};
	size_t currentAnimationIndex_ = 0;

	std::unique_ptr<Object3d> object_;
	std::unique_ptr<Skeleton> skeleton_;
	SkinCluster skinCluster_{};
#ifdef _DEBUG
	std::unique_ptr<Object3d> debugBox_;
#endif // _DEBUG
	Camera* camera_ = nullptr;

public:
	Boss();
	~Boss() = default;

	void Initialize(Camera* camera, const Vector3& position);
	void Update();
	void Draw();

	void SetCamera(Camera* camera) { camera_ = camera; }
	bool GetIsAlive() const { return isAlive_; }
	Vector3 GetPosition() const { return transform_.translate; }
	Vector3 GetScale() const { return transform_.scale; }
	void SetPosition(const Vector3& position) { transform_.translate = position; }

	void SetHPSubtract(int hp);
	bool CanTakeDamage() const { return damageInvincibleTimer_ <= 0.0f; }
	void TriggerDamageInvincibility() { damageInvincibleTimer_ = damageInvincibleDuration_; }
	int GetLastSkillDamageId() const { return lastSkillDamageId_; }
	void SetLastSkillDamageId(int skillDamageId) { lastSkillDamageId_ = skillDamageId; }
};