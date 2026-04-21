#pragma once
#include "Mesh/Object3d/Object3d.h"
#include <memory>
class BurningObject {

	std::unique_ptr<Object3d> obj_ = nullptr;

public:
	BurningObject();
	void Initialize();
	void Update();
	void Draw();

};
