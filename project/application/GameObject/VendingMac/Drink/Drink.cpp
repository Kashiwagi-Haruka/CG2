#include "Drink.h"
#include "Camera.h"
#include "Function.h"
#include "Object3d/Object3dCommon.h"
#include <Model/ModelManager.h>
#include <random>

namespace {
constexpr int kMaxDrinkCount = 10;
const Vector3 kDrinkPositionOffset = {0.0f, 0.9f, 0.45f};
const Vector3 kDrinkScale = {1.0f, 1.0f, 1.0f};
constexpr float kDrinkSpeed = 0.12f;
constexpr float kMoveOffset = 0.6f;
constexpr float kGroundY = 0.0f;
} // namespace

Drink::Drink() {
	ModelManager::GetInstance()->LoadModel("Resources/TD3_3102/3d/Drink/water", "water");
	ModelManager::GetInstance()->LoadModel("Resources/TD3_3102/3d/Drink/tea", "tea");
	ModelManager::GetInstance()->LoadModel("Resources/TD3_3102/3d/Drink/energyDrink", "energyDrink");
	ModelManager::GetInstance()->LoadModel("Resources/TD3_3102/3d/Drink/Severed_Head", "Severed_Head");
	ModelManager::GetInstance()->LoadModel("Resources/TD3_3102/3d/coffee", "coffee");
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
		obj->SetModel("coffee");
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
	return obj;
}

void Drink::SpawnDrink(DrinkName type) {
	static std::mt19937 randomEngine{std::random_device{}()};
	static std::uniform_real_distribution<float> random01(0.4f, 1.6f);
	static std::uniform_real_distribution<float> randomSign(-1.0f, 1.0f);

	SpawnedDrink spawned{};
	spawned.type = type;
	spawned.object = CreateDrinkObject(type);
	spawned.object->SetTranslate(vendingMacPosition_ + kDrinkPositionOffset);

	const Vector3 randomVector = {randomSign(randomEngine), random01(randomEngine), randomSign(randomEngine)};
	spawned.moveVector = randomVector * (kMoveOffset * kDrinkSpeed);
	spawned.isGrounded = false;
	spawnedDrinks_.push_back(std::move(spawned));
}

void Drink::Update() {
	for (auto& spawnedDrink : spawnedDrinks_) {
		if (!spawnedDrink.object) {
			continue;
		}

		if (!spawnedDrink.isGrounded) {
			Vector3 translate = spawnedDrink.object->GetTranslate();
			translate += spawnedDrink.moveVector;
			if (translate.y <= kGroundY) {
				translate.y = kGroundY;
				spawnedDrink.isGrounded = true;
			}
			spawnedDrink.object->SetTranslate(translate);
		}
		spawnedDrink.object->Update();
	}
}

void Drink::Draw() {
	Object3dCommon::GetInstance()->DrawCommon();
	for (const auto& spawnedDrink : spawnedDrinks_) {
		if (spawnedDrink.object) {
			spawnedDrink.object->Draw();
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