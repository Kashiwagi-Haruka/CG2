#pragma once
#include <memory>
#include "Primitive/Primitive.h"
#include "Transform.h"
class SavePoint {
	Transform transform_ = {};

	public:

	~SavePoint();
	void Initialize();
	void Update();
	void Draw();

	void Save();

	void SetTransform(const Transform& transform) { transform_ = transform; }

};
