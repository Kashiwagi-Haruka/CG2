#include "Poster.h"

#include "Camera.h"
#include "Function.h"

#include <cassert>
#include <numbers>
Poster::Poster() {
	for (auto& poster : posters_) {
		poster = std::make_unique<Primitive>();
	}

	localTransforms_[0] = {
	    .scale = {0.9f,   1.2f,                 1.0f },
	    .rotate = {0.0f,  std::numbers::pi_v<float>*-0.5f, 0.0f },
	    .translate = {-1.41f, 1.35f,                0.55f},
	};

	localTransforms_[1] = {
	    .scale = {0.9f,  1.2f,                  1.0f },
	    .rotate = {0.0f,  std::numbers::pi_v<float>*0.5f, 0.0f },
	    .translate = {1.41f, 1.35f,                 0.55f},
	};
}

void Poster::Initialize() {
	for (auto& poster : posters_) {
		poster->Initialize(Primitive::Plane, "Resources/TD3_3102/2d/atHome.jpg");
		poster->SetEnableLighting(false);
		poster->SetColor({1.0f, 1.0f, 1.0f, 1.0f});
	}
}

void Poster::Update() {
	assert(parentMat_);
	for (size_t i = 0; i < posters_.size(); ++i) {
		Matrix4x4 localMat = Function::MakeAffineMatrix(localTransforms_[i].scale, localTransforms_[i].rotate, localTransforms_[i].translate);
		Matrix4x4 worldMat = Function::Multiply(localMat, *parentMat_);
		posters_[i]->SetWorldMatrix(worldMat);
		posters_[i]->Update();
	}
}

void Poster::Draw() {
	for (auto& poster : posters_) {
		poster->Draw();
	}
}

void Poster::SetCamera(Camera* camera) {
	for (auto& poster : posters_) {
		poster->SetCamera(camera);
		poster->UpdateCameraMatrices();
	}
}