#pragma once
#include <memory>
#include "Primitive/Primitive.h"
#include "Transform.h"
class SavePoint {

	std::unique_ptr<Primitive> savePoint_ = nullptr;
	Transform transform_ = {};




	public:

	~SavePoint();
	void Initialize();
	void Update();
	void Draw();

	void Save();



};
