#include "VendingDrinkEmitter.h"
#include "Model/ModelManager.h"
#include <algorithm>
#include <cmath>

void VendingDrinkEmitter::Initialize() {
	ModelManager::GetInstance()->LoadModel("Resources/TD3_3102/3d/Coffee", "Coffee");
	instances_.clear();
	instances_.resize(kPoolSize);
	for (auto& instance : instances_) {
		instance.object = std::make_unique<Object3d>();
		instance.object->SetModel("Coffee");
		instance.object->Initialize();
		instance.object->SetTransform({
			{kScale, kScale, kScale},
			{0.0f, 0.0f, 0.0f},
			{0.0f, -1000.0f, 0.0f}
		});
		instance.isActive = false;
		instance.velocity = {0.0f, 0.0f, 0.0f};
		instance.lifeTime = 0.0f;
	}
}

void VendingDrinkEmitter::SetCamera(Camera* camera) {
	for (auto& instance : instances_) {
		instance.object->SetCamera(camera);
		instance.object->UpdateCameraMatrices();
	}
}

void VendingDrinkEmitter::SetSpawn(const Vector3& origin, const Vector3& launchDirection) {
	spawnOrigin_ = origin;
	Vector3 horizontalDirection = {launchDirection.x, 0.0f, launchDirection.z};
	const float lengthSq = horizontalDirection.x * horizontalDirection.x + horizontalDirection.z * horizontalDirection.z;
	if (lengthSq <= 1e-6f) {
		launchDirection_ = {0.0f, 0.0f, 1.0f};
	} else {
		const float invLength = 1.0f / std::sqrt(lengthSq);
		launchDirection_ = {horizontalDirection.x * invLength, 0.0f, horizontalDirection.z * invLength};
	}
}

void VendingDrinkEmitter::SpawnSingle() {
	auto it = std::find_if(instances_.begin(), instances_.end(), [](const DrinkInstance& drink) { return !drink.isActive; });
	if (it == instances_.end()) {
		it = instances_.begin();
	}

	const uint32_t spawnIndex = static_cast<uint32_t>(std::distance(instances_.begin(), it));
	const float seed = static_cast<float>(spawnIndex) * 1.6180339f;
	const float angle = seed * 6.2831853f;
	const float spreadRatio = std::fmod(seed * 0.37f, 1.0f) * 2.0f - 1.0f;
	const Vector3 right = {-launchDirection_.z, 0.0f, launchDirection_.x};

	it->isActive = true;
	it->lifeTime = kLifeTime;
	it->velocity = {
		launchDirection_.x * kLaunchSpeed + right.x * spreadRatio * kLaunchSpread,
		kLaunchVerticalSpeed + std::abs(spreadRatio) * 0.25f,
		launchDirection_.z * kLaunchSpeed + right.z * spreadRatio * kLaunchSpread,
	};
	it->object->SetTransform({
		{kScale, kScale, kScale},
		{0.0f, angle, 0.0f},
		spawnOrigin_
	});
}

void VendingDrinkEmitter::Update(float deltaTime) {
	for (auto& instance : instances_) {
		if (!instance.isActive) {
			continue;
		}

		auto transform = instance.object->GetTransform();
		instance.velocity.y += kGravity * deltaTime;
		transform.translate += instance.velocity * deltaTime;
		transform.rotate.x += instance.velocity.z * 0.05f * deltaTime;
		transform.rotate.z -= instance.velocity.x * 0.05f * deltaTime;
		if (transform.translate.y < 0.25f) {
			transform.translate.y = 0.25f;
			instance.velocity = {0.0f, 0.0f, 0.0f};
		}

		instance.lifeTime -= deltaTime;
		if (instance.lifeTime <= 0.0f) {
			instance.isActive = false;
			transform.translate = {0.0f, -1000.0f, 0.0f};
		}

		instance.object->SetTransform(transform);
		instance.object->Update();
	}
}

void VendingDrinkEmitter::Draw() {
	for (auto& instance : instances_) {
		if (instance.isActive) {
			instance.object->Draw();
		}
	}
}
