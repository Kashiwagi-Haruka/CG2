#pragma once
#include <Object3d/Object3d.h>
#include <memory>

class Camera;

class Drink {
public:
	enum DrinkName { WATER, TEA, COFFEE, ENERGY, SeveredHead, kMaxDrinkNameCount };

	Drink();
	void Initialize();
	void Update();
	void Draw();
	void SetCamera(Camera* camera);

	// true だったら大量排出のトリガー
	bool ChangeDrink();
	void SetVendingMacPosition(const Vector3& position) { vendingMacPosition_ = position; }

private:
	Object3d* GetCurrentDrinkObject();

	DrinkName currentDrinkName_ = WATER;
	Vector3 vendingMacPosition_ = {0.0f, 0.0f, 0.0f};
	int drinkCount_ = 0;

	std::unique_ptr<Object3d> waterObj_;
	std::unique_ptr<Object3d> teaObj_;
	std::unique_ptr<Object3d> coffeeObj_;
	std::unique_ptr<Object3d> energyDrinkObj_;
	std::unique_ptr<Object3d> severedHeadObj_;
};