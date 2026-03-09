#pragma once
#include "Object3d/InstancedObject3d/InstancedObject3d.h"
#include <memory>
#include <vector>

class Camera;

class Coffee {
public:
	Coffee();
	void Initialize();
	void Update(Camera* camera, const Vector3& lightDirection);
	void Draw();
	uint32_t GetInstanceCount() const;

private:
	std::unique_ptr<InstancedObject3d> instancedObject_ = nullptr;
	std::vector<Vector3> offsets_{};
	std::vector<float> velocitiesY_{};
	std::vector<float> collisionRadius_{};
};