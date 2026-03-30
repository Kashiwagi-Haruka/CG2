#include "Elevator.h"

#include "Camera.h"
#include "Function.h"
#include "GameObject/YoshidaMath/YoshidaMath.h"
#include "Model/ModelManager.h"
#include "Object3d/Object3dCommon.h"
#include <cmath>

Elevator::Elevator() {
	ModelManager::GetInstance()->LoadGltfModel("Resources/TD3_3102/3d/Elevator", "Elevator");

	modelObj_ = std::make_unique<Object3d>();
	modelObj_->SetModel("Elevator");

	floorBox_ = std::make_unique<Primitive>();
}

void Elevator::Initialize() {
	modelObj_->Initialize();

	floorBox_->Initialize(Primitive::PrimitiveName::Box);
	floorBox_->SetEnableLighting(true);
	floorBox_->SetColor({0.15f, 0.15f, 0.15f, 1.0f});

	elevatorTransform_ = {
	    .scale = {2.0f,   2.0f,          2.0f },
	    .rotate = {0.0f,   Function::kPi, 0.0f },
	    .translate = {-20.0f, baseHeight_,   -3.5f},
	};

	floorBoxTransform_ = {
	    .scale = {4.2f,	                       0.2f,               4.2f                          },
	    .rotate = {0.0f,	                       0.0f,               0.0f                          },
	    .translate = {elevatorTransform_.translate.x, baseHeight_ - 0.8f, elevatorTransform_.translate.z},
	};

	animationTime_ = 0.0f;
}

void Elevator::SetCamera(Camera* camera) {
	modelObj_->SetCamera(camera);
	modelObj_->UpdateCameraMatrices();

	floorBox_->SetCamera(camera);
	floorBox_->UpdateCameraMatrices();
}

void Elevator::Update() {
	animationTime_ += YoshidaMath::kDeltaTime * animationSpeed_;
	

	floorBoxTransform_.translate.x = elevatorTransform_.translate.x;
	floorBoxTransform_.translate.z = elevatorTransform_.translate.z;
	floorBoxTransform_.translate.y = elevatorTransform_.translate.y - 0.8f;

	modelObj_->SetTransform(elevatorTransform_);
	modelObj_->Update();

	floorBox_->SetTransform(floorBoxTransform_);
	floorBox_->Update();
}

void Elevator::Draw() {
	Object3dCommon::GetInstance()->DrawCommonSkinning();
	modelObj_->Draw();
	Object3dCommon::GetInstance()->DrawCommon();
	floorBox_->Draw();
}