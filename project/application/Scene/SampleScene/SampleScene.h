#pragma once
#include "BaseScene.h"
#include "Object3d.h"
#include "Camera.h"
#include "Light/DirectionalLight.h"
#include <memory>
#include "Transform.h"
class SampleScene : public BaseScene{

	std::unique_ptr<Object3d> uvBallObj_ = nullptr;
	std::unique_ptr<Camera> camera_ = nullptr;
	DirectionalLight directionalLight_{};

	Transform cameraTransform_{};

	public:
	SampleScene();
	~SampleScene() override = default;
	void Initialize() override;
	void Update() override;
	void Draw() override;
	void Finalize() override;


};
