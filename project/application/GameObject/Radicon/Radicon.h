#pragma once
#include <memory>
#include "Engine/Texture/Mesh/Object3d/Object3d.h"
#include"GameObject/YoshidaMath/CollisionManager/Collider.h"
class Camera;
class Radicon:public YoshidaMath::Collider {

	std::unique_ptr<Object3d> obj_;
	Camera* camera_ = nullptr;
	Transform transform_;
	Vector3 velocity_;
	float speed_;

	public:

	void Initialize();
	void SetCamera(Camera* camera);
	void Update(bool isOperationMode);
	void Draw();
	void SetTransform(const Transform& transform) { transform_ = transform; }
	const Transform& GetTransform() const { return transform_; }
	

};
