#include "Drink.h"
#include "Camera.h"
#include "Function.h"
#include "Object3d/Object3dCommon.h"
#include <Model/ModelManager.h>

namespace {
constexpr int kMaxDrinkCount = 10;
const Vector3 kDrinkPositionOffset = {0.0f, 0.9f, 0.45f};
const Vector3 kDrinkScale = {1.0f, 1.0f, 1.0f};
} // namespace

Drink::Drink() {
	ModelManager::GetInstance()->LoadModel("Resources/TD3_3102/3d/Drink/water", "water");
	ModelManager::GetInstance()->LoadModel("Resources/TD3_3102/3d/Drink/tea", "tea");
	ModelManager::GetInstance()->LoadModel("Resources/TD3_3102/3d/Drink/energyDrink", "energyDrink");
	ModelManager::GetInstance()->LoadModel("Resources/TD3_3102/3d/Drink/Severed_Head", "Severed_Head");
	ModelManager::GetInstance()->LoadModel("Resources/TD3_3102/3d/coffee", "coffee");

	waterObj_ = std::make_unique<Object3d>();
	teaObj_ = std::make_unique<Object3d>();
	energyDrinkObj_ = std::make_unique<Object3d>();
	severedHeadObj_ = std::make_unique<Object3d>();
	coffeeObj_ = std::make_unique<Object3d>();
}

void Drink::Initialize() {
	waterObj_->SetModel("water");
	waterObj_->Initialize();
	teaObj_->SetModel("tea");
	teaObj_->Initialize();
	energyDrinkObj_->SetModel("energyDrink");
	energyDrinkObj_->Initialize();
	severedHeadObj_->SetModel("Severed_Head");
	severedHeadObj_->Initialize();
	coffeeObj_->SetModel("coffee");
	coffeeObj_->Initialize();
}

bool Drink::ChangeDrink() {
	++drinkCount_;
	if (drinkCount_ >= kMaxDrinkCount) {
		drinkCount_ = kMaxDrinkCount;
		return true;
	}
	currentDrinkName_ = static_cast<DrinkName>((static_cast<int>(currentDrinkName_) + 1) % kMaxDrinkNameCount);
	return false;
}

Object3d* Drink::GetCurrentDrinkObject() {
	switch (currentDrinkName_) {
	case Drink::WATER:
		return waterObj_.get();
	case Drink::TEA:
		return teaObj_.get();
	case Drink::COFFEE:
		return coffeeObj_.get();
	case Drink::ENERGY:
		return energyDrinkObj_.get();
	case Drink::SeveredHead:
		return severedHeadObj_.get();
	default:
		return nullptr;
	}
}

void Drink::Update() {
	Object3d* currentDrink = GetCurrentDrinkObject();
	if (!currentDrink) {
		return;
	}

	currentDrink->SetTranslate(vendingMacPosition_ + kDrinkPositionOffset);
	currentDrink->SetScale(kDrinkScale);
	currentDrink->Update();
}

void Drink::Draw() {
	Object3dCommon::GetInstance()->DrawCommon();
	Object3d* currentDrink = GetCurrentDrinkObject();
	if (currentDrink) {
		currentDrink->Draw();
	}
}

void Drink::SetCamera(Camera* camera) {
	waterObj_->SetCamera(camera);
	teaObj_->SetCamera(camera);
	coffeeObj_->SetCamera(camera);
	energyDrinkObj_->SetCamera(camera);
	severedHeadObj_->SetCamera(camera);
}