#include "RadiconStage.h"
#include "Engine/math/Function.h"
#include "GameObject/YoshidaMath/CollisionManager/CollisionManager.h"
#include "GameObject/GameCamera/PlayerCamera/PlayerCamera.h"
#include "Engine/Editor/EditorTool/Hierarchy/Hierarchy.h"
#include "Primitive/Primitive.h"
#include "GameObject/Player/Player.h"
#include <cmath>

RadiconStage::RadiconStage(Player* player) : player_(player) {
	radicon_ = std::make_unique<Radicon>();
	testField_ = std::make_unique<TestField>();
	operationChangeBox_ = std::make_unique<OperationChangeBox>();
}

void RadiconStage::Initialize() { 
	Hierarchy::GetInstance()->BeginRegisterFile("RadiconStage_objectEditors.json");
	testField_->Initialize();
	radicon_->Initialize();
	operationChangeBox_->Initialize();
	radicon_->SetTransform({
	    .scale = {0.01f, 0.01f, 0.01f},
	    .rotate = {0.0f, Function::kPi, 0.0f},
	    .translate = {0.0f, 0.17f, 3.5f},
	});

	for (auto& primitive : roomPrimitives_) {
		primitive = std::make_unique<Primitive>();
		primitive->Initialize(Primitive::Box, "Resources/TD3_3102/2d/white.png");
		primitive->SetEnableLighting(true);
		primitive->SetColor({0.9f, 0.9f, 0.95f, 1.0f});
	}

	roomPrimitives_[0]->SetTransform({.scale = {12.0f, 0.2f, 12.0f}, .rotate = {0.0f}, .translate = {0.0f, -0.11f, 0.0f}});
	roomPrimitives_[1]->SetTransform({.scale = {12.0f, 4.0f, 0.2f}, .rotate = {0.0f}, .translate = {0.0f, 2.0f, -6.0f}});
	roomPrimitives_[2]->SetTransform({.scale = {12.0f, 4.0f, 0.2f}, .rotate = {0.0f}, .translate = {0.0f, 2.0f, 6.0f}});
	roomPrimitives_[3]->SetTransform({.scale = {0.2f, 4.0f, 12.0f}, .rotate = {0.0f}, .translate = {-6.0f, 2.0f, 0.0f}});
	roomPrimitives_[4]->SetTransform({.scale = {0.2f, 4.0f, 12.0f}, .rotate = {0.0f}, .translate = {6.0f, 2.0f, 0.0f}});

	Hierarchy::GetInstance()->EndRegisterFile();
}

void RadiconStage::SetPlayer(Player* player) { player_ = player; }

void RadiconStage::SetCollisionManager([[maybe_unused]] CollisionManager* collisionManager) {


}

void RadiconStage::UpdateGameObject([[maybe_unused]] Camera* camera, [[maybe_unused]] const Vector3& lightDirection, [[maybe_unused]] Player* player) { 
	testField_->Update();
	operationChangeBox_->Update();

	if (!isOperationMode_ && operationChangeBox_->ConsumeInteractRequest() && playerCamera_ && player_) {
		isOperationMode_ = true;
		lockedPlayerPosition_ = player_->GetTransform().translate;

		const Vector3 forward = operationChangeBox_->GetForward();
		Transform operationCameraTransform{};
		operationCameraTransform.scale = {1.0f, 1.0f, 1.0f};
		operationCameraTransform.translate = operationChangeBox_->GetTransform().translate - forward * 1.7f + Vector3{0.0f, 0.65f, 0.0f};
		operationCameraTransform.rotate.x = 0.17f;
		operationCameraTransform.rotate.y = std::atan2(forward.x, forward.z);
		operationCameraTransform.rotate.z = 0.0f;
		playerCamera_->EnableFixedTransform(operationCameraTransform);
	}

	if (isOperationMode_ && player_) {
		player_->SetTranslate(lockedPlayerPosition_);
	}

	radicon_->Update(isOperationMode_);
	for (auto& primitive : roomPrimitives_) {
		primitive->Update();
	}
}

void RadiconStage::UpdatePortal() {/*記載なし*/}

void RadiconStage::CheckCollision() {}

void RadiconStage::DrawModel([[maybe_unused]] bool isShadow, [[maybe_unused]] bool drawPortal, [[maybe_unused]] bool isDrawParticle) {
	testField_->Draw();
	for (auto& primitive : roomPrimitives_) {
		primitive->Draw();
	}
	operationChangeBox_->Draw();
	radicon_->Draw();
}

void RadiconStage::SetSceneCameraForDraw([[maybe_unused]] Camera* camera) {
	testField_->SetCamera(camera);
	radicon_->SetCamera(camera);
	operationChangeBox_->SetCamera(camera);
	for (auto& primitive : roomPrimitives_) {
		primitive->SetCamera(camera);
		primitive->UpdateCameraMatrices();
	}
}

void RadiconStage::SetPlayerCamera([[maybe_unused]] PlayerCamera* playerCamera) {
	playerCamera_ = playerCamera;
	radicon_->SetCamera(playerCamera->GetCamera());
	operationChangeBox_->SetPlayerCamera(playerCamera);
}

PortalManager* RadiconStage::GetPortalManager() { return nullptr; }

void RadiconStage::SetLightManager(Yoshida::LightManager* lightManager) { lightManager_ = lightManager; }

bool RadiconStage::CheckHitPlayerByStageHazard([[maybe_unused]] const Vector3& playerPosition, [[maybe_unused]] float playerRadius, [[maybe_unused]] float minHitSpeed) const { return false; }
