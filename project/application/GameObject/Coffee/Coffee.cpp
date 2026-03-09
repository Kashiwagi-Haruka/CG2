#include "Coffee.h"
#include "Model/ModelManager.h"
#include <cmath>

namespace {
constexpr const char* kCoffeeModelDirectory = "Resources/TD3_3102/3d/Coffee";
constexpr const char* kCoffeeModelName = "Coffee";
constexpr float kCoffeeStartHeight = 3.0f;
constexpr float kCoffeeGroundY = 0.0f;
constexpr float kCoffeeGravity = -0.015f;
constexpr float kCoffeeBounceDamping = 0.8f;
constexpr float kCoffeeSeparationBias = 0.001f;
constexpr uint32_t kCoffeeInstanceCount = 1000;
constexpr uint32_t kCoffeeSpawnColumns = 4;
constexpr float kCoffeeSpawnSpacingX = 2.4f;
constexpr float kCoffeeSpawnSpacingZ = 2.8f;
constexpr float kCoffeeMinScale = 0.22f;
constexpr float kCoffeeScaleStep = 0.04f;
} // namespace

Coffee::Coffee() { instancedObject_ = std::make_unique<InstancedObject3d>(); }

void Coffee::Initialize() {
	ModelManager::GetInstance()->LoadModel(kCoffeeModelDirectory, kCoffeeModelName);
	instancedObject_->Initialize(kCoffeeModelName);
	instancedObject_->SetSpawnOrigin({0.0f, 0.0f, 0.0f});
	instancedObject_->SetInstanceCount(kCoffeeInstanceCount);

	offsets_.resize(kCoffeeInstanceCount);
	velocitiesY_.assign(kCoffeeInstanceCount, 0.0f);
	collisionRadius_.resize(kCoffeeInstanceCount);

	const float columnOffset = (static_cast<float>(kCoffeeSpawnColumns) - 1.0f) * 0.5f;
	const uint32_t rowCount = (kCoffeeInstanceCount + kCoffeeSpawnColumns - 1u) / kCoffeeSpawnColumns;
	const float rowOffset = (static_cast<float>(rowCount) - 1.0f) * 0.5f;
	for (uint32_t i = 0; i < kCoffeeInstanceCount; ++i) {
		const uint32_t column = i % kCoffeeSpawnColumns;
		const uint32_t row = i / kCoffeeSpawnColumns;
		const float scale = kCoffeeMinScale + static_cast<float>(i % 4u) * kCoffeeScaleStep;
		const Vector3 instanceScale = {scale, scale, scale};
		instancedObject_->SetInstanceScale(i, instanceScale);

		const float x = (static_cast<float>(column) - columnOffset) * kCoffeeSpawnSpacingX;
		const float z = (static_cast<float>(row) - rowOffset) * kCoffeeSpawnSpacingZ;
		const float y = kCoffeeStartHeight + static_cast<float>((i * 5u) % 9u) * 0.25f;
		offsets_[i] = {x, y, z};
		collisionRadius_[i] = 0.2f + scale * 0.35f;
		instancedObject_->SetInstanceOffset(i, offsets_[i]);
	}
}

void Coffee::Update(Camera* camera, const Vector3& lightDirection) {
	for (size_t i = 0; i < offsets_.size(); ++i) {
		velocitiesY_[i] += kCoffeeGravity;
		offsets_[i].y += velocitiesY_[i];
		if (offsets_[i].y <= kCoffeeGroundY) {
			offsets_[i].y = kCoffeeGroundY;
			velocitiesY_[i] = -velocitiesY_[i] * kCoffeeBounceDamping;
		}
	}

	for (size_t i = 0; i < offsets_.size(); ++i) {
		for (size_t j = i + 1; j < offsets_.size(); ++j) {
			Vector3 delta = {
			    offsets_[j].x - offsets_[i].x,
			    0.0f,
			    offsets_[j].z - offsets_[i].z,
			};
			const float distanceSq = delta.x * delta.x + delta.z * delta.z;
			const float minDistance = collisionRadius_[i] + collisionRadius_[j];
			const float minDistanceSq = minDistance * minDistance;
			if (distanceSq >= minDistanceSq) {
				continue;
			}

			float distance = std::sqrt(distanceSq);
			Vector3 separationDir = {1.0f, 0.0f, 0.0f};
			if (distance > 0.0001f) {
				separationDir = {delta.x / distance, 0.0f, delta.z / distance};
			} else {
				distance = 0.0f;
			}

			const float pushAmount = ((minDistance - distance) * 0.5f) + kCoffeeSeparationBias;
			offsets_[i].x -= separationDir.x * pushAmount;
			offsets_[i].z -= separationDir.z * pushAmount;
			offsets_[j].x += separationDir.x * pushAmount;
			offsets_[j].z += separationDir.z * pushAmount;
		}
	}

	for (size_t i = 0; i < offsets_.size(); ++i) {
		instancedObject_->SetInstanceOffset(i, offsets_[i]);
	}

	instancedObject_->Update(camera, lightDirection);
}

void Coffee::Draw() { instancedObject_->Draw(); }

uint32_t Coffee::GetInstanceCount() const { return instancedObject_->GetInstanceCount(); }