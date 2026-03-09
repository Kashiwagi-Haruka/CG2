#include "CoffeeScene.h"
#include "Input.h"
#include "Light/DirectionalLight.h"
#include "Model/ModelManager.h"
#include "Object3d/Object3dCommon.h"
#ifdef USE_IMGUI
#include <imgui.h>
#endif // USE_IMGUI
#include <cmath>
#include <numbers>

namespace {
constexpr float kRoomWidth = 9.0f;
constexpr float kRoomHeight = 2.4f;
constexpr float kRoomDepth = 15.0f;
constexpr const char* kCoffeeModelDirectory = "Resources/TD3_3102/3d/Coffee";
constexpr const char* kCoffeeModelName = "Coffee";
constexpr float kCoffeeStartHeight = 3.0f;
constexpr float kCoffeeGroundY = 0.0f;
constexpr float kCoffeeGravity = -0.015f;
constexpr float kCoffeeBounceDamping = 0.8f;
constexpr float kCoffeeSeparationBias = 0.001f;
constexpr uint32_t kCoffeeInstanceCount = 1000;
constexpr uint32_t kCoffeeSpawnColumns = 4;
constexpr float kCoffeeSpawnSpacingX = 2.4f;
constexpr float kCoffeeSpawnSpacingZ = 2.8f;
constexpr float kCoffeeMinScale = 0.22f;
constexpr float kCoffeeScaleStep = 0.04f;
} // namespace


CoffeeScene::CoffeeScene() {
	for (auto& wall : roomWalls_) {
		wall = std::make_unique<Primitive>();
	}
	camera_ = std::make_unique<Camera>();
	debugCamera_ = std::make_unique<DebugCamera>();
	coffeeInstancedObject_ = std::make_unique<InstancedObject3d>();
	cameraTransform_ = {
	    .scale = {1.0f, 1.0f, 1.0f},
	    .rotate = {0.0f, 0.0f, 0.0f},
	    .translate = {0.0f, 1.5f, -5.0f},
	};
}

void CoffeeScene::Initialize() {
	const float halfWidth = kRoomWidth * 0.5f;
	const float halfHeight = kRoomHeight * 0.5f;
	const float halfDepth = kRoomDepth * 0.5f;

	Object3dCommon::GetInstance()->SetRandomNoiseEnabled(false);

	camera_->SetTransform(cameraTransform_);
	debugCamera_->Initialize();
	debugCamera_->SetTranslation(cameraTransform_.translate);

	for (auto& wall : roomWalls_) {
		wall->Initialize(Primitive::Plane);
		wall->SetCamera(camera_.get());
		wall->SetEnableLighting(true);
		wall->SetColor({0.95f, 0.95f, 0.95f, 1.0f});
	}

	roomWalls_[0]->SetScale({kRoomWidth, kRoomDepth, 1.0f});
	roomWalls_[0]->SetRotate({std::numbers::pi_v<float> * 0.5f, 0.0f, 0.0f});
	roomWalls_[0]->SetTranslate({0.0f, 0.0f, 0.0f});
	roomWalls_[1]->SetScale({kRoomWidth, kRoomDepth, 1.0f});
	roomWalls_[1]->SetRotate({-std::numbers::pi_v<float> * 0.5f, 0.0f, 0.0f});
	roomWalls_[1]->SetTranslate({0.0f, kRoomHeight, 0.0f});
	roomWalls_[2]->SetScale({kRoomWidth, kRoomHeight, 1.0f});
	roomWalls_[2]->SetRotate({0.0f, 0.0f, 0.0f});
	roomWalls_[2]->SetTranslate({0.0f, halfHeight, halfDepth});
	roomWalls_[3]->SetScale({kRoomWidth, kRoomHeight, 1.0f});
	roomWalls_[3]->SetRotate({0.0f, std::numbers::pi_v<float>, 0.0f});
	roomWalls_[3]->SetTranslate({0.0f, halfHeight, -halfDepth});
	roomWalls_[4]->SetScale({kRoomDepth, kRoomHeight, 1.0f});
	roomWalls_[4]->SetRotate({0.0f, -std::numbers::pi_v<float> * 0.5f, 0.0f});
	roomWalls_[4]->SetTranslate({-halfWidth, halfHeight, 0.0f});
	roomWalls_[5]->SetScale({kRoomDepth, kRoomHeight, 1.0f});
	roomWalls_[5]->SetRotate({0.0f, std::numbers::pi_v<float> * 0.5f, 0.0f});
	roomWalls_[5]->SetTranslate({halfWidth, halfHeight, 0.0f});

	ModelManager::GetInstance()->LoadModel(kCoffeeModelDirectory, kCoffeeModelName);
	coffeeInstancedObject_->Initialize(kCoffeeModelName);
	coffeeInstancedObject_->SetSpawnOrigin({0.0f, 0.0f, 0.0f});
	coffeeInstancedObject_->SetInstanceCount(kCoffeeInstanceCount);

	coffeeOffsets_.resize(kCoffeeInstanceCount);
	coffeeVelocitiesY_.assign(kCoffeeInstanceCount, 0.0f);
	coffeeCollisionRadius_.resize(kCoffeeInstanceCount);

	const float columnOffset = (static_cast<float>(kCoffeeSpawnColumns) - 1.0f) * 0.5f;
	const uint32_t rowCount = (kCoffeeInstanceCount + kCoffeeSpawnColumns - 1u) / kCoffeeSpawnColumns;
	const float rowOffset = (static_cast<float>(rowCount) - 1.0f) * 0.5f;
	for (uint32_t i = 0; i < kCoffeeInstanceCount; ++i) {
		const uint32_t column = i % kCoffeeSpawnColumns;
		const uint32_t row = i / kCoffeeSpawnColumns;
		const float scale = kCoffeeMinScale + static_cast<float>(i % 4u) * kCoffeeScaleStep;
		const Vector3 instanceScale = {scale, scale, scale};
		coffeeInstancedObject_->SetInstanceScale(i, instanceScale);

		const float x = (static_cast<float>(column) - columnOffset) * kCoffeeSpawnSpacingX;
		const float z = (static_cast<float>(row) - rowOffset) * kCoffeeSpawnSpacingZ;
		const float y = kCoffeeStartHeight + static_cast<float>((i * 5u) % 9u) * 0.25f;
		coffeeOffsets_[i] = {x, y, z};
		coffeeCollisionRadius_[i] = 0.2f + scale * 0.35f;
		coffeeInstancedObject_->SetInstanceOffset(i, coffeeOffsets_[i]);
	}
}


void CoffeeScene::Update() {
	auto* input = Input::GetInstance();
	if (input->TriggerKey(DIK_F1)) {
		useDebugCamera_ = !useDebugCamera_;
	}

#ifdef USE_IMGUI
	if (ImGui::Begin("CoffeeCamera")) {
		ImGui::Checkbox("Use Debug Camera (F1)", &useDebugCamera_);
		ImGui::Text("Debug: LMB drag rotate, Shift+LMB drag pan, Wheel zoom");
		ImGui::Text("Instanced Coffee Count: %u", coffeeInstancedObject_->GetInstanceCount());
		if (ImGui::TreeNode("Transform")) {
			if (!useDebugCamera_) {
				ImGui::DragFloat3("Scale", &cameraTransform_.scale.x, 0.01f);
				ImGui::DragFloat3("Rotate", &cameraTransform_.rotate.x, 0.01f);
				ImGui::DragFloat3("Translate", &cameraTransform_.translate.x, 0.01f);
			}
			ImGui::TreePop();
		}
	}
	ImGui::End();
#endif // USE_IMGUI

	for (size_t i = 0; i < coffeeOffsets_.size(); ++i) {
		coffeeVelocitiesY_[i] += kCoffeeGravity;
		coffeeOffsets_[i].y += coffeeVelocitiesY_[i];
		if (coffeeOffsets_[i].y <= kCoffeeGroundY) {
			coffeeOffsets_[i].y = kCoffeeGroundY;
			coffeeVelocitiesY_[i] = -coffeeVelocitiesY_[i] * kCoffeeBounceDamping;
		}
	}

	for (size_t i = 0; i < coffeeOffsets_.size(); ++i) {
		for (size_t j = i + 1; j < coffeeOffsets_.size(); ++j) {
			Vector3 delta = {
			    coffeeOffsets_[j].x - coffeeOffsets_[i].x,
			    0.0f,
			    coffeeOffsets_[j].z - coffeeOffsets_[i].z,
			};
			const float distanceSq = delta.x * delta.x + delta.z * delta.z;
			const float minDistance = coffeeCollisionRadius_[i] + coffeeCollisionRadius_[j];
			const float minDistanceSq = minDistance * minDistance;
			if (distanceSq >= minDistanceSq) {
				continue;
			}

			float distance = std::sqrt(distanceSq);
			Vector3 separationDir = {1.0f, 0.0f, 0.0f};
			if (distance > 0.0001f) {
				separationDir = {delta.x / distance, 0.0f, delta.z / distance};
			} else {
				distance = 0.0f;
			}

			const float pushAmount = ((minDistance - distance) * 0.5f) + kCoffeeSeparationBias;
			coffeeOffsets_[i].x -= separationDir.x * pushAmount;
			coffeeOffsets_[i].z -= separationDir.z * pushAmount;
			coffeeOffsets_[j].x += separationDir.x * pushAmount;
			coffeeOffsets_[j].z += separationDir.z * pushAmount;
		}
	}

	for (size_t i = 0; i < coffeeOffsets_.size(); ++i) {
		coffeeInstancedObject_->SetInstanceOffset(i, coffeeOffsets_[i]);
	}

	if (useDebugCamera_) {
		debugCamera_->Update();
		camera_->SetViewProjectionMatrix(debugCamera_->GetViewMatrix(), debugCamera_->GetProjectionMatrix());
	} else {
		camera_->SetTransform(cameraTransform_);
		camera_->Update();
	}

	for (auto& wall : roomWalls_) {
		wall->SetCamera(camera_.get());
		wall->Update();
	}
	Object3dCommon::GetInstance()->SetDefaultCamera(camera_.get());
	DirectionalLight directionalLight{};
	directionalLight.color = {1.0f, 1.0f, 1.0f, 1.0f};
	directionalLight.direction = {0.1f, -0.5f, -0.2f};
	directionalLight.intensity = 1.0f;
	Object3dCommon::GetInstance()->SetDirectionalLight(directionalLight);
	coffeeInstancedObject_->Update(camera_.get(), directionalLight.direction);
}


void CoffeeScene::Draw() {
	Object3dCommon::GetInstance()->DrawCommon();
	//for (auto& wall : roomWalls_) {
	//	wall->Draw();
	//}
	coffeeInstancedObject_->Draw();
}

void CoffeeScene::Finalize() {}