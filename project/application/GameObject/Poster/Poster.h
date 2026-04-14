#pragma once
#include "Engine/Texture/Mesh/Primitive/Primitive.h"
#include "Engine/math/Transform.h"
#include <array>
#include <memory>

class Camera;

class Poster {
public:
	Poster();
	void Initialize();
	void Update();
	void Draw();
	void SetCamera(Camera* camera);
	void SetParentMat(const Matrix4x4* parentMat) { parentMat_ = parentMat; }

private:
	std::array<std::unique_ptr<Primitive>, 2> posters_{};
	std::array<Transform, 2> localTransforms_{};
	const Matrix4x4* parentMat_ = nullptr;
};