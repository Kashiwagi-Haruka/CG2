#pragma once
#include "Vector3.h"
#include "Transform.h"
class Player {

	enum class State {
		kIdle,
		kRunning,
		kJumping,
		kFalling, 
		kAttacking

	};

	Vector3 velocity_;
	
	Transform transform_;



	public:

	void Initialize();
	void Update();
	void Draw();

};
