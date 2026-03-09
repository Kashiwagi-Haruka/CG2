#define NOMINMAX
#include "Coffee.h"
#include "DirectXCommon.h"
#include "Model/ModelManager.h"
#include "Object3d/Object3dCommon.h"
#include <algorithm>
#include <cmath>
#include <cstdint>

namespace {
constexpr const char* kCoffeeModelDirectory = "Resources/TD3_3102/3d/Coffee";
constexpr const char* kCoffeeModelName = "Coffee";
constexpr uint32_t kCoffeeInstanceCount = 10000;
constexpr uint32_t kCoffeeInitialVisibleCount = 100;
constexpr Vector3 kCoffeeSpawnOrigin = {0.0f, 5.0f, 0.0f};
constexpr float kCoffeeMinScale = 0.22f;
constexpr float kCoffeeScaleStep = 0.0f;
constexpr float kCoffeeGravity = -15.0f;
constexpr float kCoffeeBounceDamping = 0.0f;
constexpr float kCoffeeGroundFriction = 0.93f;
constexpr float kCoffeeCollisionDamping = 0.85f;
constexpr float kCoffeeSeparationBias = 0.001f;
constexpr float kCoffeeMinSpawnInterval = 0.001f;
constexpr float kCoffeeMaxSpawnInterval = 0.005f;
constexpr float kCoffeeSpawnAreaRadius = 0.35f;
constexpr float kCoffeeSpatialCellSize = 0.6f;
constexpr float kCoffeeCanTopY = 0.85f;
constexpr float kCoffeeCanTopRadius = 0.7f;

int64_t HashCell(int32_t x, int32_t y, int32_t z) { return (static_cast<int64_t>(x) << 42) ^ (static_cast<int64_t>(y) << 21) ^ static_cast<int64_t>(z); }

struct CellCoord {
	int32_t x;
	int32_t y;
	int32_t z;
};

struct CellEntry {
	int64_t key;
	uint32_t index;
};

struct CellRange {
	int64_t key;
	uint32_t begin;
	uint32_t end;
};

CellCoord ComputeCellCoord(const Vector3& position) {
	return {
	    static_cast<int32_t>(std::floor(position.x / kCoffeeSpatialCellSize)),
	    static_cast<int32_t>(std::floor(position.y / kCoffeeSpatialCellSize)),
	    static_cast<int32_t>(std::floor(position.z / kCoffeeSpatialCellSize)),
	};
}
} // namespace

Coffee::Coffee() { instancedObject_ = std::make_unique<InstancedObject3d>(); }

void Coffee::Initialize() {
	ModelManager::GetInstance()->LoadModel(kCoffeeModelDirectory, kCoffeeModelName);
	instancedObject_->Initialize(kCoffeeModelName);
	instancedObject_->SetSpawnOrigin({0.0f, 0.0f, 0.0f});
	renderedInstanceCapacity_ = std::min(kCoffeeInitialVisibleCount, kCoffeeInstanceCount);
	instancedObject_->SetInstanceCount(renderedInstanceCapacity_);

	instances_.resize(kCoffeeInstanceCount);
	activeInstanceCount_ = 0;
	spawnTimer_ = 0.0f;
	nextSpawnInterval_ = kCoffeeMaxSpawnInterval;

	for (uint32_t i = 0; i < kCoffeeInstanceCount; ++i) {
		const float scale = kCoffeeMinScale + static_cast<float>(i % 4u) * kCoffeeScaleStep;
		instancedObject_->SetInstanceScale(i, {scale, scale, scale});

		instances_[i].position = kCoffeeSpawnOrigin;
		instances_[i].scale = scale;
		instances_[i].radius = 0.2f + scale * 0.35f;
		instances_[i].velocity = {0.0f, 0.0f, 0.0f};
		instances_[i].isActive = false;
		instancedObject_->SetInstanceOffset(i, {0.0f, -1000.0f, 0.0f});
	}

	simulationParams_.deltaTime = 1.0f / 60.0f;
	simulationParams_.floorY = 0.0f;
	simulationParams_.canTopCenter = {0.0f, 0.0f, 0.0f};
	simulationParams_.canTopY = kCoffeeCanTopY;
	simulationParams_.canTopRadius = kCoffeeCanTopRadius;
	simulationParams_.gravity = kCoffeeGravity;
	simulationParams_.bounceDamping = kCoffeeBounceDamping;
	simulationParams_.separationBias = kCoffeeSeparationBias;
	simulationParams_.roomMinX = -100.0f;
	simulationParams_.roomMaxX = 100.0f;
	simulationParams_.roomMinZ = -800.0f;
	simulationParams_.roomMaxZ = 800.0f;
}

void Coffee::RunSimulation() {
	if (instances_.empty()) {
		return;
	}

	const float deltaTime = simulationParams_.deltaTime;
	const float gravity = simulationParams_.gravity;
	const float floorY = simulationParams_.floorY;
	const float bounceDamping = simulationParams_.bounceDamping;
	const Vector3 canTopCenter = simulationParams_.canTopCenter;
	const float canTopY = simulationParams_.canTopY;
	const float canTopRadius = simulationParams_.canTopRadius;
	const float separationBias = simulationParams_.separationBias;
	const float roomMinX = simulationParams_.roomMinX;
	const float roomMaxX = simulationParams_.roomMaxX;
	const float roomMinZ = simulationParams_.roomMinZ;
	const float roomMaxZ = simulationParams_.roomMaxZ;

	spawnTimer_ += deltaTime;
	while (spawnTimer_ >= nextSpawnInterval_ && activeInstanceCount_ < static_cast<uint32_t>(instances_.size())) {
		spawnTimer_ -= nextSpawnInterval_;
		const uint32_t spawnIndex = activeInstanceCount_;
		const float t = static_cast<float>(spawnIndex) / static_cast<float>(instances_.size() - 1u);
		nextSpawnInterval_ = kCoffeeMaxSpawnInterval + (kCoffeeMinSpawnInterval - kCoffeeMaxSpawnInterval) * t;

		auto& spawnInstance = instances_[spawnIndex];
		const float seed = static_cast<float>(spawnIndex) * 1.6180339f;
		const float angle = seed * 6.2831853f;
		const float radial = std::fmod(seed * 0.73f, 1.0f) * kCoffeeSpawnAreaRadius;
		spawnInstance.position.x = kCoffeeSpawnOrigin.x + std::cos(angle) * radial;
		spawnInstance.position.z = kCoffeeSpawnOrigin.z + std::sin(angle) * radial;
		spawnInstance.position.y = kCoffeeSpawnOrigin.y;
		spawnInstance.velocity = {0.0f, 0.0f, 0.0f};
		spawnInstance.isActive = true;
		++activeInstanceCount_;
		EnsureInstanceCapacity(activeInstanceCount_);
		instancedObject_->SetInstanceOffset(spawnIndex, spawnInstance.position);
	}

	static std::vector<Vector3> pendingPush;
	static std::vector<CellCoord> cellCoords;
	static std::vector<CellEntry> cellEntries;
	static std::vector<CellRange> cellRanges;

	pendingPush.resize(activeInstanceCount_);
	std::fill(pendingPush.begin(), pendingPush.end(), Vector3{0.0f, 0.0f, 0.0f});

	cellCoords.resize(activeInstanceCount_);
	cellEntries.resize(activeInstanceCount_);

	for (size_t i = 0; i < activeInstanceCount_; ++i) {
		auto& instance = instances_[i];
		instance.velocity.y += gravity * deltaTime;
		instance.position.x += instance.velocity.x * deltaTime;
		instance.position.y += instance.velocity.y * deltaTime;
		instance.position.z += instance.velocity.z * deltaTime;
		const float offsetXFromCan = instance.position.x - canTopCenter.x;
		const float offsetZFromCan = instance.position.z - canTopCenter.z;
		const float horizontalDistSqFromCan = offsetXFromCan * offsetXFromCan + offsetZFromCan * offsetZFromCan;
		const float canRadius = std::max(canTopRadius - instance.radius, 0.0f);
		const bool isInsideCanTop = horizontalDistSqFromCan <= canRadius * canRadius;

		if (isInsideCanTop && instance.position.y <= canTopY) {
			instance.position.y = canTopY;
			instance.velocity.y = 0.0f;
			instance.velocity.x *= kCoffeeGroundFriction;
			instance.velocity.z *= kCoffeeGroundFriction;
		} else if (instance.position.y <= floorY) {
			instance.position.y = floorY;
			instance.velocity.y = 0.0f;
			instance.velocity.x *= kCoffeeGroundFriction;
			instance.velocity.z *= kCoffeeGroundFriction;
		}
	}

	for (uint32_t i = 0; i < activeInstanceCount_; ++i) {
		const CellCoord coord = ComputeCellCoord(instances_[i].position);
		cellCoords[i] = coord;
		cellEntries[i] = {HashCell(coord.x, coord.y, coord.z), i};
	}

	std::sort(cellEntries.begin(), cellEntries.end(), [](const CellEntry& lhs, const CellEntry& rhs) { return lhs.key < rhs.key; });
	cellRanges.clear();
	cellRanges.reserve(cellEntries.size());
	for (uint32_t begin = 0; begin < cellEntries.size();) {
		const int64_t key = cellEntries[begin].key;
		uint32_t end = begin + 1;
		while (end < cellEntries.size() && cellEntries[end].key == key) {
			++end;
		}
		cellRanges.push_back({key, begin, end});
		begin = end;
	}

	const auto findCellRange = [](const std::vector<CellRange>& cellRanges, int64_t key) -> const CellRange* {
		auto it = std::lower_bound(cellRanges.begin(), cellRanges.end(), key, [](const CellRange& range, int64_t value) { return range.key < value; });
		if (it == cellRanges.end() || it->key != key) {
			return nullptr;
		}
		return &(*it);
	};

	for (size_t i = 0; i < activeInstanceCount_; ++i) {
		const CellCoord coord = cellCoords[i];
		for (int32_t dz = -1; dz <= 1; ++dz) {
			for (int32_t dy = -1; dy <= 1; ++dy) {
				for (int32_t dx = -1; dx <= 1; ++dx) {
					const CellRange* range = findCellRange(cellRanges, HashCell(coord.x + dx, coord.y + dy, coord.z + dz));
					if (!range) {
						continue;
					}
					for (uint32_t entryIndex = range->begin; entryIndex < range->end; ++entryIndex) {
						const uint32_t j = cellEntries[entryIndex].index;
						if (j <= i) {
							continue;
						}
						const float minDist = instances_[i].radius + instances_[j].radius;
						const Vector3 delta = {
						    instances_[i].position.x - instances_[j].position.x,
						    instances_[i].position.y - instances_[j].position.y,
						    instances_[i].position.z - instances_[j].position.z,
						};
						const float distSq = delta.x * delta.x + delta.y * delta.y + delta.z * delta.z;
						const float minDistSq = minDist * minDist;

						if (distSq < minDistSq && distSq > 1e-7f) {
							const float dist = std::sqrt(distSq);
							const float overlap = minDist - dist;
							const float scale = (overlap * 0.5f + separationBias) / dist;
							const Vector3 normal = {delta.x / dist, delta.y / dist, delta.z / dist};
							const Vector3 push = {delta.x * scale, delta.y * scale, delta.z * scale};
							pendingPush[i].x += push.x;
							pendingPush[i].y += push.y;
							pendingPush[i].z += push.z;
							pendingPush[j].x -= push.x;
							pendingPush[j].y -= push.y;
							pendingPush[j].z -= push.z;

							const Vector3 relative = {
							    instances_[i].velocity.x - instances_[j].velocity.x,
							    instances_[i].velocity.y - instances_[j].velocity.y,
							    instances_[i].velocity.z - instances_[j].velocity.z,
							};
							const float relativeAlongNormal = relative.x * normal.x + relative.y * normal.y + relative.z * normal.z;

							if (relativeAlongNormal < 0.0f) {
								const float impulse = -(1.0f + kCoffeeCollisionDamping) * relativeAlongNormal * 0.5f;
								const Vector3 impulseVec = {normal.x * impulse, normal.y * impulse, normal.z * impulse};
								instances_[i].velocity.x += impulseVec.x;
								instances_[i].velocity.y += impulseVec.y;
								instances_[i].velocity.z += impulseVec.z;
								instances_[j].velocity.x -= impulseVec.x;
								instances_[j].velocity.y -= impulseVec.y;
								instances_[j].velocity.z -= impulseVec.z;
							}
						}
					}
				}
			}
		}
	}

	for (size_t i = 0; i < activeInstanceCount_; ++i) {
		auto& instance = instances_[i];
		instance.position.x = std::clamp(instance.position.x + pendingPush[i].x, roomMinX, roomMaxX);
		instance.position.y += pendingPush[i].y;
		instance.position.z = std::clamp(instance.position.z + pendingPush[i].z, roomMinZ, roomMaxZ);
		const float offsetXFromCan = instance.position.x - canTopCenter.x;
		const float offsetZFromCan = instance.position.z - canTopCenter.z;
		const float horizontalDistSqFromCan = offsetXFromCan * offsetXFromCan + offsetZFromCan * offsetZFromCan;
		const float canRadius = std::max(canTopRadius - instance.radius, 0.0f);
		const bool isInsideCanTop = horizontalDistSqFromCan <= canRadius * canRadius;
		if (isInsideCanTop && instance.position.y < canTopY) {
			instance.position.y = canTopY;
			instance.velocity.y = std::max(instance.velocity.y, 0.0f);
		} else if (instance.position.y < floorY) {
			instance.position.y = floorY;
			instance.velocity.y = std::max(instance.velocity.y, 0.0f);
		}
		if (instance.position.x <= roomMinX || instance.position.x >= roomMaxX) {
			instance.velocity.x *= -kCoffeeCollisionDamping;
		}
		if (instance.position.z <= roomMinZ || instance.position.z >= roomMaxZ) {
			instance.velocity.z *= -kCoffeeCollisionDamping;
		}
		instancedObject_->SetInstanceOffset(i, instance.position);
	}
}
void Coffee::EnsureInstanceCapacity(uint32_t requiredCount) {
	if (requiredCount <= renderedInstanceCapacity_) {
		return;
	}

	uint32_t newCapacity = std::max(requiredCount, std::min(kCoffeeInstanceCount, std::max(renderedInstanceCapacity_ * 2u, 1u)));
	newCapacity = std::min(newCapacity, kCoffeeInstanceCount);
	const uint32_t previousCapacity = renderedInstanceCapacity_;
	renderedInstanceCapacity_ = newCapacity;
	instancedObject_->SetInstanceCount(renderedInstanceCapacity_);

	for (uint32_t i = 0; i < renderedInstanceCapacity_; ++i) {
		const float scale = instances_[i].scale;
		instancedObject_->SetInstanceScale(i, {scale, scale, scale});
		if (i < activeInstanceCount_ && instances_[i].isActive) {
			instancedObject_->SetInstanceOffset(i, instances_[i].position);
		} else if (i >= previousCapacity) {
			instancedObject_->SetInstanceOffset(i, {0.0f, -1000.0f, 0.0f});
		}
	}
}
void Coffee::Update(Camera* camera, const Vector3& lightDirection) {
	simulationParams_.deltaTime = std::max(Object3dCommon::GetInstance()->GetDxCommon()->GetDeltaTime(), 1.0f / 120.0f);
	RunSimulation();
	instancedObject_->Update(camera, lightDirection);
}

void Coffee::Draw() { instancedObject_->Draw(); }

uint32_t Coffee::GetInstanceCount() const { return activeInstanceCount_; }