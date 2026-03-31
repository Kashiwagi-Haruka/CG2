#pragma once
#include "Object3d/Object3d.h"
#include "Primitive/Primitive.h"
#include <memory>

class Camera;

class Elevator {
public:
	Elevator();

	void Initialize();
	void SetCamera(Camera* camera);
	void Update();
	void Draw();

private:
	std::unique_ptr<Object3d> modelObj_ = nullptr;
	std::unique_ptr<Primitive> floorBox_ = nullptr;

	Transform elevatorTransform_{};
	Transform floorBoxTransform_{};

	float animationTime_ = 0.0f;
	float animationSpeed_ = 1.2f;
	float animationAmplitude_ = 0.35f;
	float baseHeight_ = 2.4f;
};