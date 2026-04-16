#pragma once
#include "Object3d/InstancedObject3d/InstancedObject3d.h"
#include <memory>
#include <vector>

class Camera;

class Coffees {
public:
	Coffees();
	~Coffees();
	void Initialize();
	void Update(Camera* camera, const Vector3& lightDirection);
	void Draw();
	uint32_t GetInstanceCount() const;
	void SetSpawnOrigin(const Vector3& spawnOrigin);
	void SetLaunchDirection(const Vector3& launchDirection);
	void SetFloorY(float floorY);
	void SetSpawnContainment(const Vector3& center, float topY, float radius);
	void StartSpill();
	bool CheckHitPlayer(const Vector3& playerPosition, float playerRadius, float minHitSpeed) const;

	void SetRoomBounds(float minX, float maxX, float minZ, float maxZ);

private:
	struct InstanceData {
		Vector3 position;
		Vector3 velocity = {0.0f, 0.0f, 0.0f};
		Vector3 rotation = {0.0f, 0.0f, 0.0f};
		Vector3 angularVelocity = {0.0f, 0.0f, 0.0f};
		float scale = 0.5f;
		float radius = 0.25f;
		float halfHeight = 0.15f;
		bool isActive = false;
		float padding = 0.0f;
	};

	struct SimulationParams {
		float deltaTime = 1.0f / 60.0f;
		float roomMinX = -10.0f;
		float roomMaxX = 10.0f;
		float roomMinZ = -10.0f;
		float roomMaxZ = 10.0f;
		float floorY = 0.0f;
		Vector3 canTopCenter = {0.0f, 0.0f, 0.0f};
		float canTopY = 0.0f;
		float canTopRadius = 0.0f;
		float gravity = -1.0f;
		float bounceDamping = 0.8f;
		float separationBias = 0.001f;
	};

	void RunSimulation();
	void EnsureInstanceCapacity(uint32_t requiredCount);

	std::unique_ptr<InstancedObject3d> instancedObject_ = nullptr;
	std::vector<InstanceData> instances_{};

	SimulationParams simulationParams_{};
	uint32_t activeInstanceCount_ = 0;
	uint32_t renderedInstanceCapacity_ = 0;
	float spawnTimer_ = 0.0f;
	float nextSpawnInterval_ = 0.0f;
	Vector3 spawnOrigin_ = {0.0f, 5.0f, 0.0f};
	Vector3 launchDirection_ = {0.0f, 0.0f, 1.0f};
	bool isSpilling_ = false;
};