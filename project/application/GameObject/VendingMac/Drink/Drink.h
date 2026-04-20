#pragma once
#include <Object3d/Object3d.h>
#include <memory>
#include <vector>

class Camera;
class PlayerCamera;

class Drink {
public:
	enum DrinkName { WATER, TEA, COFFEE, ENERGY, SeveredHead, kMaxDrinkNameCount };

	Drink();
	void Initialize();
	void Update();
	void Draw();
	void SetCamera(Camera* camera);
	void SetPlayerCamera(PlayerCamera* playerCamera) { playerCamera_ = playerCamera; }

	// true だったら大量排出のトリガー
	bool ChangeDrink();
	void SetVendingMacPosition(const Vector3& position) { vendingMacPosition_ = position; }
	void SetVendingMacForward(const Vector3& forward) { vendingMacForward_ = forward; }

private:
	struct SpawnedDrink {
		DrinkName type = WATER;
		std::unique_ptr<Object3d> object = nullptr;
		Vector3 velocity = {0.0f, 0.0f, 0.0f};
		bool isGrounded = false;
		bool isRayHit = false;
	};

	void SpawnDrink(DrinkName type);
	std::unique_ptr<Object3d> CreateDrinkObject(DrinkName type) const;

	DrinkName currentDrinkName_ = WATER;
	Vector3 vendingMacPosition_ = {0.0f, 0.0f, 0.0f};
	Vector3 vendingMacForward_ = {0.0f, 0.0f, 1.0f};
	int drinkCount_ = 0;
	std::vector<SpawnedDrink> spawnedDrinks_;
	Camera* camera_ = nullptr;
	PlayerCamera* playerCamera_ = nullptr;
};