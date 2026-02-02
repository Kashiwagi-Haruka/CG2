#include "ExpCube.h"
#include "Camera.h"

namespace {
const Vector4 kExpCubeColor = {0.6f, 1.0f, 0.3f, 1.0f};
const float kRotateSpeed = 0.03f;
} // namespace

void ExpCube::Initialize(Camera* camera, const Vector3& position) {
	camera_ = camera;
	isCollected_ = false;
	primitive_ = std::make_unique<Primitive>();
	primitive_->Initialize(Primitive::Box, "Resources/2d/ExpGauge.png");
	primitive_->SetEnableLighting(false);
	primitive_->SetColor(kExpCubeColor);
	transform_ = {
	    {0.35f,      0.35f,      0.35f     },
	    {0.0f,       0.0f,       0.0f      },
	    {position.x, position.y, position.z},
	};
	primitive_->SetTransform(transform_);
}

void ExpCube::Update() {
	if (isCollected_) {
		return;
	}

	transform_.rotate.y += kRotateSpeed;
	primitive_->SetCamera(camera_);
	primitive_->SetTransform(transform_);
	primitive_->Update();
}

void ExpCube::Draw() {
	if (isCollected_) {
		return;
	}
	primitive_->Draw();
}