#pragma once
#include "Engine/Texture/Mesh/Object3d/Object3d.h"
#include "Engine/math/Transform.h"
class Poster {

	Object3d obj_;

public: 
	Poster();
	void Initialize();
	void Update();
	void Draw();
};
