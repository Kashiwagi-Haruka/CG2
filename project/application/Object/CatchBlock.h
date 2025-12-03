#pragma once
#include "Object3d.h"
#include "Transform.h"
#include "Camera.h"
class CatchBlock {

	Object3d* object3d_;
	Transform transform_;
	Camera* camera_;



public:
	
	CatchBlock();
	~CatchBlock();
	void Initialize();
	void Update();
	void Draw();
	void SetCamera(Camera* camera);


};
