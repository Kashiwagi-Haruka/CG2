#pragma once
#include "Object3d.h"
#include "Transform.h"
#include <memory>
#include "Camera.h"
class PlayerSword {

	std::unique_ptr<Object3d> swordObject_;
	Camera* camera = nullptr;

	#ifdef _DEBUG
	std::unique_ptr<Object3d> debugBox_;
	#endif // _DEBUG


	public:

		PlayerSword();
		void Initialize();
		void Update(const Transform& playerTransform);
		void Draw();
	    void SetCamera(Camera* cam) { camera = cam;}

};
