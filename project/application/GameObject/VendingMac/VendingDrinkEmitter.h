#pragma once

#include "Object3d/Object3d.h"
#include <memory>
#include <vector>

class Camera;

class VendingDrinkEmitter {
public:
	struct DrinkInstance {
		std::unique_ptr<Object3d> object = nullptr;
		Vector3 velocity = {0.0f, 0.0f, 0.0f};
		float lifeTime = 0.0f;
		bool isActive = false;
	};

	void Initialize();
	void SetCamera(Camera* camera);
	void SetSpawn(const Vector3& origin, const Vector3& launchDirection);
	void SpawnSingle();
	void Update(float deltaTime);
	void Draw();

private:
	static constexpr uint32_t kPoolSize = 16;
	static constexpr float kScale = 0.22f;
	static constexpr float kLifeTime = 6.0f;
	static constexpr float kGravity = -15.0f;
	static constexpr float kLaunchSpeed = 8.5f;
	static constexpr float kLaunchVerticalSpeed = 1.8f;
	static constexpr float kLaunchSpread = 1.2f;

	std::vector<DrinkInstance> instances_{};
	Vector3 spawnOrigin_ = {0.0f, 0.0f, 0.0f};
	Vector3 launchDirection_ = {0.0f, 0.0f, 1.0f};
};
