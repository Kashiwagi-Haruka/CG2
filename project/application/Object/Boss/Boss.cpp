#define NOMINMAX
#include "Boss.h"
#include "Camera.h"
#include "Function.h"
#include "Object3d/Object3d.h"
#include "Vector4.h"
#include <algorithm>
#include <cmath>
#include <numbers>
#include "Object3d/Object3dCommon.h"

namespace {
const Vector4 kDamageInvincibleColor = {1.0f, 0.25f, 0.25f, 1.0f};
const Vector4 kDefaultColor = {1.0f, 1.0f, 1.0f, 1.0f};
} // namespace

Boss::Boss() { object_ = std::make_unique<Object3d>(); }

void Boss::Initialize(Camera* camera, const Vector3& position) {
	hp_ = 50;
	isAlive_ = true;
	damageInvincibleTimer_ = 0.0f;
	lastSkillDamageId_ = -1;
	animationTimer_ = 0.0f;
	appearTimer_ = 0.0f;

	camera_ = camera;
	object_->Initialize();
	object_->SetModel("WaterBoss");
	animationClips_ = Animation::LoadAnimationClips("Resources/3d", "WaterBoss");
	if (!animationClips_.empty()) {
		currentAnimationIndex_ = 0;
		for (size_t i = 0; i < animationClips_.size(); ++i) {
			if (animationClips_[i].name == "Walk") {
				currentAnimationIndex_ = i;
				break;
			}
		}
		object_->SetAnimation(&animationClips_[currentAnimationIndex_], true);
		object_->ResetAnimationTime();
	}
	basePosition_ = position;
	baseScale_ = {1.0f, 1.0f, 1.0f};
	transform_ = {
	    .scale{0.0f, 0.0f, 0.0f},
	    .rotate{0.0f, 0.0f, 0.0f},
	    .translate = position,
	};
	object_->SetTransform(transform_);
	object_->SetCamera(camera_);
	object_->SetColor(kDefaultColor);
	object_->Update();
}

void Boss::Update() {
	if (!isAlive_) {
		return;
	}
	const float deltaTime = 1.0f / 60.0f;
	animationTimer_ += deltaTime;
	if (damageInvincibleTimer_ > 0.0f) {
		damageInvincibleTimer_ -= deltaTime;
		if (damageInvincibleTimer_ < 0.0f) {
			damageInvincibleTimer_ = 0.0f;
		}
	}

	if (appearTimer_ < appearDuration_) {
		appearTimer_ += deltaTime;
	}
	float appearT = std::clamp(appearTimer_ / appearDuration_, 0.0f, 1.0f);
	float smoothT = appearT * appearT * (3.0f - 2.0f * appearT);
	float scalePulse = 1.0f + std::sin(animationTimer_ * std::numbers::pi_v<float>) * 0.05f;
	transform_.scale = baseScale_ * (smoothT * scalePulse);

	float bob = std::sin(animationTimer_ * std::numbers::pi_v<float> * 2.0f) * 0.5f;
	transform_.translate = basePosition_;
	transform_.translate.y += bob;
	transform_.translate.x += std::sin(animationTimer_ * std::numbers::pi_v<float> * 0.5f) * 0.3f;
	transform_.rotate.y += deltaTime * 0.5f;
	transform_.rotate.z = std::sin(animationTimer_ * std::numbers::pi_v<float>) * 0.05f;

	object_->SetCamera(camera_);
	object_->SetTransform(transform_);
	object_->SetColor(damageInvincibleTimer_ > 0.0f ? kDamageInvincibleColor : kDefaultColor);
	object_->Update();
}

void Boss::Draw() {
	if (!isAlive_) {
		return;
	}
	Object3dCommon::GetInstance()->DrawCommonSkinning();
	object_->Draw();
}

void Boss::SetHPSubtract(int hp) {
	hp_ -= hp;
	if (hp_ <= 0) {
		isAlive_ = false;
	}
}