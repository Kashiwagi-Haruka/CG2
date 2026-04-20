#pragma once
#include <memory>
#include "Engine/Texture/Mesh/Object3d/Object3d.h"
class Camera;
class Radicon {

	std::unique_ptr<Object3d> obj_;
	
	Vector3 velocity_;
	float speed_;

	public:

	void Initialize();
	void SetCamera(Camera* camera);
	void Update();
	void Draw();
	

};
