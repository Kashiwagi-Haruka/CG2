#pragma once
#include "Object3d.h"
#include "Transform.h"
#include "Camera.h"
#include <memory>
class CatchBlock {

	std::unique_ptr<Object3d> object3d_;
	Transform transform_;
	Camera* camera_;



public:
	
	CatchBlock();
	
	void Initialize();
	void Update();
	void Draw();
	void SetCamera(Camera* camera);


};
