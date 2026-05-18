#pragma once
#include "Engine/Texture/Mesh/Primitive/Primitive.h"
#include <memory>
#include "transform.h"
class Camera;
class ElevatorNumber {

	std::unique_ptr<Primitive> primitive_;
	Camera* camera_ = nullptr;
	Transform transform_;
	Transform elevatorTransform_ = {
	    {1.0f, 1.0f, 1.0f},
	    {0.0f, 0.0f, 0.0f},
	    {0.0f, 0.0f, 0.0f},
	};
	;
	
	int number_ = 0;
	public:
	void Initialize();
	void Update();
	void Draw();
	void SetStageNumber(int number) { number_ = number; }
	void SetCamera(Camera* camera);
	void SetElevatorTransform(Transform transform) { elevatorTransform_ = transform; }
	
};