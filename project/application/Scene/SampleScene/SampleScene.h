#pragma once
#include "BaseScene.h"
#include "Object3d.h"
#include "Camera.h"
#include "Light/DirectionalLight.h"
#include <memory>
#include "Transform.h"
class SampleScene : public BaseScene{

	std::unique_ptr<Object3d> uvBallObj_ = nullptr;
	std::unique_ptr<Object3d> fieldObj_ = nullptr;
	std::unique_ptr<Camera> camera_ = nullptr;
	DirectionalLight directionalLight_{};
	Transform uvBallTransform_{};
	Transform cameraTransform_{};
	PointLight pointLight_{};
	Vector4 color = {1.0f, 1.0f, 1.0f, 1.0f};
	bool enableLighting = true;
	float shininess = 5.0f;

	public:
	SampleScene();
	~SampleScene() override = default;
	void Initialize() override;
	void Update() override;
	void Draw() override;
	void Finalize() override;


};
