#pragma once
#include "Animation/Animation.h"
#include "Animation/Skeleton.h"
#include "Animation/SkinCluster.h"
#include "ElevatorNumber.h"
#include "GameObject/Door/AutoLockSystem.h"
#include "GameObject/Poster/Poster.h"
#include "GameObject/Wall/Wall.h"
#include "Object3d/Object3d.h"
#include "Primitive/Primitive.h"
#include "Text/EleveterNumberText/EleveterNumberText.h"
#include <GameObject/GameCamera/PlayerCamera/PlayerCamera.h>
#include"GameObject/Wall/Wall.h"
#include"GameObject/YoshidaMath/CollisionManager/ObjectCollider.h"
#include <array>
#include <memory>
#include"Light/CommonLight/PointCommonLight.h"
#include"Light/CommonLight/AreaCommonLight.h"

class Camera;

class Elevator {
public:

	Elevator();
	~Elevator();
	void Initialize();
	void SetCamera(Camera* camera);
	void Update();
	void Draw();
    void DrawSprite();
	void SetPlayerCamera(PlayerCamera* camera) { playerCamera_ = camera; };
	void SetStageNumber(int stageNumber);
    std::map<std::string, std::unique_ptr<AutoLockSystem>>& GetAutoLockSys() { return autoLockSystems_; };
	bool IsSceneTransitionStart() const { return isSceneTransitionStart_; }
  std::unordered_map<std::string, std::unique_ptr<ObjectCollider>>& GetColliders() { return colliders_; }
  std::array<PointCommonLight, 2>& GetPointLights() { return pointLights_; };
  AreaCommonLight& GetAreaLight() { return areaLight_; }
  void SetAreaLightColor(const Vector4 color) { areaLight_.color = color; };
private:
    //ライトの位置を更新する
    void UpdateLightPos();
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
    std::array<PointCommonLight, 2> pointLights_;
    AreaCommonLight areaLight_;
    float lightPosY_ = 0.0f;
    float lightVelocity_ = 2.0f;
    // 新しい状態管理
    bool isPlayerInside_ = false;
    float insideTimer_ = 0.0f;
    const float insideOpenDelay_ = 5.0f; // 中に入って5秒

    Matrix4x4 worldMat_;
    static PlayerCamera* playerCamera_;
    std::unique_ptr<Object3d> modelObj_ = nullptr;
    // アニメーション
    Animation::AnimationData blendedPoseAnimation_{};
    // 骨
    std::unique_ptr<Skeleton> skeleton_{};
    // スキン
    SkinCluster skinCluster_{};

	std::unique_ptr<ElevatorNumber> elevatorNumber_ = nullptr;
	int stageNumber_ = 0;
	EleveterNumberText elevatorNumberText_{};

	std::string animationGroupName_ = "Elevator";
	const float kAnimationBlendDuration_ = 0.0f;
	bool animationFinished_ = false;
	std::string desiredAnimationName = "Idle";
	static bool isRayHit_;
    Matrix4x4 doorMatrixLeft_;
    Matrix4x4 doorMatrixRight_;
	Transform elevatorTransform_;

    float baseHeight_ = 0.0f;

    // オートロック
    std::map<std::string ,std::unique_ptr<AutoLockSystem>> autoLockSystems_;

    Poster poster_;
    bool isSceneTransitionStart_ = false;

    bool isSceneTransition_ = false;
};