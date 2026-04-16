#pragma once
#include <memory>
#include <vector>
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
	float speed_ = 0.1f;
	std::vector<Vector3> vector_;
	int drinkCount_ = 0;

	std::vector<std::unique_ptr<Object3d>> waterObj_;
	std::vector<std::unique_ptr<Object3d>> teaObj_;
	std::vector<std::unique_ptr<Object3d>> coffeeObj_;
	std::vector<std::unique_ptr<Object3d>> energyDrinkObj_;
	std::vector<std::unique_ptr<Object3d>> severedHeadObj_;

	public:
	void Initialize();
	void Update();
	void Draw();

	//trueだったらコーヒーたくさん出てくる
	bool ChangeDrink();
	void SetVendingMacPosition(Vector3 position) { VendingMacPosition_ = position; }
};

