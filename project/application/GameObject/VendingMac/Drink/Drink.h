#pragma once
#include <memory>
#include <Object3d/Object3d.h>
class Drink {

	enum DrinkName{
		WATER,
		TEA,
		COFFEE,
		ENERGY, 
		SeveredHead, 
		kMaxDrinkNameCount
	};
	DrinkName currentDrinkName_ = WATER;

	Vector3 VendingMacPosition_ = {0.0f, 0.0f, 0.0f};
	bool isAnimationStarted_ = false;
	int drinkCount_ = 0;

	std::unique_ptr<Object3d> waterObj_;
	std::unique_ptr<Object3d> teaObj_;
	std::unique_ptr<Object3d> coffeeObj_;
	std::unique_ptr<Object3d> energyDrinkObj_;
	std::unique_ptr<Object3d> severedHeadObj_;

	public:
	void Initialize();
	void Update();
	void Draw();

	//trueだったらコーヒーたくさん出てくる
	bool ChangeDrink();
	void SetVendingMacPosition(Vector3 position) { VendingMacPosition_ = position; }
};

