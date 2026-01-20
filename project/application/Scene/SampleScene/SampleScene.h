#pragma once
#include "Animation/Animation.h"
#include "Animation/Skeleton.h"
#include "Animation/SkinCluster.h"
#include "BaseScene.h"
#include "Camera.h"
#include "DebugCamera.h"
#include "Light/AreaLight.h"
#include "Light/DirectionalLight.h"
#include "Light/PointLight.h"
#include "Light/SpotLight.h"
#include "Object3d/Object3d.h"
#include "Transform.h"
#include <array>
#include <cstdint>
#include <memory>
class SampleScene : public BaseScene {

	std::unique_ptr<Object3d> uvBallObj_ = nullptr;
	std::unique_ptr<Object3d> fieldObj_ = nullptr;
	std::unique_ptr<Object3d> planeGltf_ = nullptr;
	std::unique_ptr<Object3d> animatedCubeObj_ = nullptr;
	std::unique_ptr<Object3d> humanWalkObj_ = nullptr;
	std::unique_ptr<Object3d> humanSneakWalkObj_ = nullptr;
	std::unique_ptr<Camera> camera_ = nullptr;
	std::unique_ptr<DebugCamera> debugCamera_ = nullptr;
	bool useDebugCamera_ = false;

	DirectionalLight directionalLight_{};
	std::array<PointLight, kMaxPointLights> pointLights_{};
	uint32_t activePointLightCount_ = 0;
	std::array<SpotLight, kMaxSpotLights> spotLights_{};
	uint32_t activeSpotLightCount_ = 0;
	std::array<AreaLight, kMaxAreaLights> areaLights_{};
	uint32_t activeAreaLightCount_ = 0;

	Transform uvBallTransform_{};
	Transform cameraTransform_{};
	Transform planeGTransform_{};
	Transform animatedCubeTransform_{};
	Transform humanWalkTransform_{};
	Transform humanSneakWalkTransform_{};

	Animation::AnimationData animatedCubeAnimation_{};
	Animation::AnimationData humanWalkAnimation_{};
	Animation::AnimationData humanSneakWalkAnimation_{};
	std::unique_ptr<Skeleton> humanWalkSkeleton_{};
	std::unique_ptr<Skeleton> humanSneakWalkSkeleton_{};
	SkinCluster humanWalkSkinCluster_{};
	SkinCluster humanSneakWalkSkinCluster_{};
	float humanWalkAnimationTime_ = 0.0f;
	float humanSneakWalkAnimationTime_ = 0.0f;

	Vector4 color = {1.0f, 1.0f, 1.0f, 1.0f};
	bool enableLighting = true;
	float shininess = 40.0f;
	float environmentCoefficient = 0.25f;

public:
	SampleScene();
	~SampleScene() override = default;
	void Initialize() override;
	void Update() override;
	void Draw() override;
	void Finalize() override;
};