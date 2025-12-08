#pragma once
#include "Object3d.h"
#include "Vector3.h"
#include "Camera.h"

class House {

	Object3d* object_ = nullptr;
	Vector3 position_ = {0, 1.5f, 0};
	int hp_ = 50; // ★ house の最大HP（自由に調整可）

public:

	House();
	~House() { delete object_; }

	void Initialize(Camera* camera);

	void Update(Camera* camera);

	void Draw();

	Vector3 GetPosition() const { return position_; }
	int GetHP() const { return hp_; }
	void Damage(int amount) { hp_ -= amount; }
};
