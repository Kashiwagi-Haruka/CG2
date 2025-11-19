#pragma once
#include "Transform.h"
class Object3d;
class GameBase;
class Goal {

private:

	Transform transform_;
	Object3d* object3d_;

public:

	Goal();
	~Goal();
	void Initialize(GameBase* gameBase);
	void Update();
	void Draw();

};
