#include "Drink.h"
#include <Model/ModelManager.h>
#include "Object3d/Object3dCommon.h"
#include "Function.h"
namespace {
const int kMaxDrinkCount = 10;
const Vector3 kDrinkPositionOffset = {0.0f, 1.3f, 0.6f};
}
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
	drinkCount_++;	
	if (drinkCount_ >= kMaxDrinkCount) {
		return true;
	}
	currentDrinkName_ = static_cast<DrinkName>((static_cast<int>(currentDrinkName_) + 1) % kMaxDrinkNameCount);
	return false;
}
void Drink::Update() {
	
	switch (currentDrinkName_) {
	case Drink::WATER:
		waterObj_->Update();
		break;
	case Drink::TEA:
		teaObj_->Update();
		break;
	case Drink::COFFEE:
		coffeeObj_->Update();
		break;
	case Drink::ENERGY:
		energyDrinkObj_->Update();
		break;
	case Drink::SeveredHead:
		severedHeadObj_->Update();
		break;
	case Drink::kMaxDrinkNameCount:
		break;
	default:
		break;	
	}
}
void Drink::Draw() {
	Object3dCommon::GetInstance()->DrawCommon();
	switch (currentDrinkName_) {
	case Drink::WATER:
		waterObj_->Draw();
		break;
	case Drink::TEA:
		teaObj_->Draw();
		break;
	case Drink::COFFEE:
		coffeeObj_->Draw();
		break;
	case Drink::ENERGY:
		energyDrinkObj_->Draw();
		break;
	case Drink::SeveredHead:
		severedHeadObj_->Draw();
		break;
	case Drink::kMaxDrinkNameCount:
		break;
	default:
		break;
	}

}