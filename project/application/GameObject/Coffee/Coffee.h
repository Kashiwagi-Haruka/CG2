#pragma once
#include "Camera.h"
#include "Object3d/Object3d.h"
#include <memory>

class Coffee {

	std::unique_ptr<Object3d> coffeeObj_ = nullptr;
	Camera* camera_ = nullptr;


	public:
		Coffee();
		~Coffee() = default;
		void Initialize();
		void Update();
	    void Draw();


};
