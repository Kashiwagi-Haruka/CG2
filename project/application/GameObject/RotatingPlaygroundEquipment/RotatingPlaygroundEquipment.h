#pragma once
#include "Engine/Texture/Mesh/Object3d/Object3d.h"
#include <memory>
#include <vector>
#include "transform.h"
class Camera;
class RotatingPlaygroundEquipment {

	std::unique_ptr<Object3d> spinObj_;
	std::vector<std::unique_ptr<Object3d>> gentlemanObj_;
	Transform spinTransform_;
	std::vector<Transform> gentlemanTransform_;
	
	Camera* camera_;

public:
	RotatingPlaygroundEquipment();
	~RotatingPlaygroundEquipment();
	void Initialize();
	void Update();
	void Draw();
	void SetCamera(Camera* camera);

};
