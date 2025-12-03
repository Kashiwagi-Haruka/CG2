#pragma once
#include "Object3d.h"
#include "Camera.h"
#include "Transform.h"
class EnemyStun {

	Object3d* object_ = nullptr;
	Camera* camera_ = nullptr;
	Transform transform_;





	public:

	EnemyStun();
	~EnemyStun();
	
	void SetCamera(Camera* camera);
	void SetTranslate(Vector3 translate);
	void Initialize();
	void Update();
	void Draw();
	
};
