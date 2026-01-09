#pragma once
#include "BaseScene.h"
#include "Camera.h"
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
	std::unique_ptr<Camera> camera_ = nullptr;
	DirectionalLight directionalLight_{};
	std::array<PointLight, kMaxPointLights> pointLights_{};
	uint32_t activePointLightCount_ = 0;
	SpotLight spotLight_{};
	Transform uvBallTransform_{};
	Transform cameraTransform_{};

	Vector4 color = {1.0f, 1.0f, 1.0f, 1.0f};
	bool enableLighting = true;
	float shininess = 40.0f;

public:
	SampleScene();
	~SampleScene() override = default;
	void Initialize() override;
	void Update() override;
	void Draw() override;
	void Finalize() override;
};