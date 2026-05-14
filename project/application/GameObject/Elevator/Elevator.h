#pragma once
#include "Animation/Animation.h"
#include "Animation/Skeleton.h"
#include "Animation/SkinCluster.h"
#include "GameObject/Door/AutoLockSystem.h"
#include "GameObject/Poster/Poster.h"
#include "Object3d/Object3d.h"
#include "Primitive/Primitive.h"
#include <GameObject/GameCamera/PlayerCamera/PlayerCamera.h>
#include"GameObject/Wall/Wall.h"
#include"GameObject/YoshidaMath/CollisionManager/ObjectCollider.h"
#include <array>
#include <memory>
class Camera;

class Elevator {
public:
	Elevator();
	~Elevator();
	void Initialize();
	void SetCamera(Camera* camera);
	void Update();
	void Draw();
	void SetPlayerCamera(PlayerCamera* camera) { playerCamera_ = camera; };
	std::array<std::unique_ptr<AutoLockSystem>, 2>& GetAutoLockSys() { return autoLockSystems_; };
	bool IsSceneTransitionStart() const { return isSceneTransitionStart_; }
	std::unordered_map<std::string, std::unique_ptr<ObjectCollider>>& GetColliders() { return colliders_; }
private:
    // 当たり判定
    void CheckCollision();
	// アニメーション
	void Animation();
	//内側にいる時
	void Inside();
	void Open();
	void Close();
private:

	std::unordered_map<std::string, std::unique_ptr<ObjectCollider>>colliders_;

	// 新しい状態管理
	bool isPlayerInside_ = false;
	float insideTimer_ = 0.0f;
	const float insideOpenDelay_ = 2.0f; // 中に入って2秒

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

	Matrix4x4 doorMatrixLeft_;
	Matrix4x4 doorMatrixRight_;

	std::string animationGroupName_ = "Elevator";
	const float kAnimationBlendDuration_ = 1.0f;
	bool animationFinished_ = false;
	std::string desiredAnimationName = "Idle";
	static bool isRayHit_;

	float baseHeight_ = 0.0f;

	// オートロック
	std::array<std::unique_ptr<AutoLockSystem>, 2> autoLockSystems_;

	Poster poster_;
	bool isSceneTransitionStart_ = false;

	bool isSceneTransition_ = false;
};