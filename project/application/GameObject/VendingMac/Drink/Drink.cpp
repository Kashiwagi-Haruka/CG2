#include "Drink.h"
#include "Camera.h"
#include "DirectXCommon.h"
#include "Function.h"
#include "GameObject/GameCamera/PlayerCamera/PlayerCamera.h"
#include "GameObject/YoshidaMath/YoshidaMath.h"
#include "Object3d/Object3dCommon.h"
#include "RigidBody.h"
#include <Model/ModelManager.h>

namespace {
constexpr int kMaxDrinkCount = 10;
const Vector3 kDrinkPositionOffset = {0.0f, 0.9f, 0.45f};
const Vector3 kDrinkScale = {1.0f, 1.0f, 1.0f};
constexpr float kDrinkForwardSpeed = 0.9f;
constexpr float kInitialUpwardSpeed = 0.2f;
constexpr float kGroundY = 0.12f;
const AABB kDrinkRayAABB = {
    .min = {-0.12f, -0.12f, -0.12f},
      .max = {0.12f,  0.12f,  0.12f }
};
const Vector4 kRayHitOutlineColor = {1.0f, 1.0f, 0.0f, 1.0f};
constexpr float kRayHitOutlineWidth = 10.0f;
} // namespace


Drink::Drink() {
	ModelManager::GetInstance()->LoadModel("Resources/TD3_3102/3d/Drink/water", "water");
	ModelManager::GetInstance()->LoadModel("Resources/TD3_3102/3d/Drink/tea", "tea");
	ModelManager::GetInstance()->LoadModel("Resources/TD3_3102/3d/Drink/energyDrink", "energyDrink");
	ModelManager::GetInstance()->LoadModel("Resources/TD3_3102/3d/Drink/Severed_Head", "Severed_Head");
	ModelManager::GetInstance()->LoadModel("Resources/TD3_3102/3d/Drink/coffeeCan", "coffeeCan");
}

void Drink::Initialize() {
	spawnedDrinks_.clear();
	drinkCount_ = 0;
	currentDrinkName_ = WATER;
}

bool Drink::ChangeDrink() {
	++drinkCount_;
	if (drinkCount_ >= kMaxDrinkCount) {
		drinkCount_ = kMaxDrinkCount;
		return true;
	}

	SpawnDrink(currentDrinkName_);
	currentDrinkName_ = static_cast<DrinkName>((static_cast<int>(currentDrinkName_) + 1) % kMaxDrinkNameCount);
	return false;
}

std::unique_ptr<Object3d> Drink::CreateDrinkObject(DrinkName type) const {
	auto obj = std::make_unique<Object3d>();
	switch (type) {
	case Drink::WATER:
		obj->SetModel("water");
		break;
	case Drink::TEA:
		obj->SetModel("tea");
		break;
	case Drink::COFFEE:
		obj->SetModel("coffeeCan");
		break;
	case Drink::ENERGY:
		obj->SetModel("energyDrink");
		break;
	case Drink::SeveredHead:
		obj->SetModel("Severed_Head");
		break;
	default:
		obj->SetModel("water");
		break;
	}
	obj->Initialize();
	obj->SetScale(kDrinkScale);
	if (camera_) {
		obj->SetCamera(camera_);
	}
	obj->SetOutlineColor(kRayHitOutlineColor);
	obj->SetOutlineWidth(kRayHitOutlineWidth);
	return obj;
}

void Drink::SpawnDrink(DrinkName type) {
	SpawnedDrink spawned{};
	spawned.type = type;
	spawned.object = CreateDrinkObject(type);
	spawned.object->SetTranslate(vendingMacPosition_ + kDrinkPositionOffset);

	Vector3 forward = vendingMacForward_;
	forward.y = 0.0f;
	if (Function::Length(forward) <= 0.0001f) {
		forward = {0.0f, 0.0f, 1.0f};
	}
	forward = Function::Normalize(forward);
	spawned.velocity = forward * kDrinkForwardSpeed;
	spawned.velocity.y = kInitialUpwardSpeed;
	spawned.isGrounded = false;
	spawned.isRayHit = false;
	spawnedDrinks_.push_back(std::move(spawned));
}

void Drink::Update() {
	const float deltaTime = Object3dCommon::GetInstance()->GetDxCommon()->GetDeltaTime();
	for (auto& spawnedDrink : spawnedDrinks_) {
		if (!spawnedDrink.object) {
			continue;
		}

		Vector3 translate = spawnedDrink.object->GetTranslate();
		if (!spawnedDrink.isGrounded) {
			spawnedDrink.velocity.y -= YoshidaMath::kGravity*4.0f * deltaTime;
			translate += spawnedDrink.velocity * deltaTime;
			if (translate.y <= kGroundY) {
				translate.y = kGroundY;
				spawnedDrink.isGrounded = true;
				spawnedDrink.velocity = {0.0f, 0.0f, 0.0f};
			}
			spawnedDrink.object->SetTranslate(translate);
		}

		spawnedDrink.isRayHit = playerCamera_ ? playerCamera_->OnCollisionRay(kDrinkRayAABB, translate) : false;
		spawnedDrink.object->Update();
	}
}

void Drink::Draw() {
	for (const auto& spawnedDrink : spawnedDrinks_) {
		if (spawnedDrink.object) {
			if (spawnedDrink.isRayHit) {
				Object3dCommon::GetInstance()->DrawCommon();
				spawnedDrink.object->Draw();
				Object3dCommon::GetInstance()->DrawCommonOutline();
				spawnedDrink.object->Draw();
				Object3dCommon::GetInstance()->EndOutlineDraw();
			} else {
				Object3dCommon::GetInstance()->DrawCommon();
				spawnedDrink.object->Draw();
			}
		}
	}
}

void Drink::SetCamera(Camera* camera) {
	camera_ = camera;
	for (auto& spawnedDrink : spawnedDrinks_) {
		if (spawnedDrink.object) {
			spawnedDrink.object->SetCamera(camera);
		}
	}
}