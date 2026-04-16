#define NOMINMAX
#include "Coffees.h"
#include "Camera.h"
#include "DirectXCommon.h"
#include "Function.h"
#include "Model/ModelManager.h"
#include "Object3d/Object3dCommon.h"
#include <algorithm>
#include <cmath>
#include <cstdint>
#include"GameObject/SEManager/SEManager.h"

namespace {
    constexpr const char* kCoffeesModelDirectory = "Resources/TD3_3102/3d/Coffee";
    constexpr const char* kCoffeesModelName = "Coffee";
    constexpr uint32_t kCoffeesInstanceCount = 1000;
    constexpr uint32_t kCoffeesInitialVisibleCount = 100;
    constexpr float kCoffeesMinScale = 0.22f;
    constexpr float kCoffeesScaleStep = 0.0f;
    constexpr float kCoffeesGravity = -15.0f;
    constexpr float kCoffeesBounceDamping = 0.0f;
    constexpr float kCoffeesGroundFriction = 0.82f;
    constexpr float kCoffeesStaticFrictionSpeed = 0.07f;
    constexpr float kCoffeesCollisionDamping = 0.75f;
    constexpr float kCoffeesCollisionTangentialDamping = 0.65f;
    constexpr float kCoffeesSeparationBias = 0.001f;
    constexpr float kCoffeesMinSpawnInterval = 0.012f;
    constexpr float kCoffeesMaxSpawnInterval = 0.030f;
    constexpr float kCoffeesSpawnAreaRadius = 0.10f;
    constexpr float kCoffeesLaunchSpeed = 8.5f;
    constexpr float kCoffeesLaunchVerticalSpeed = 1.8f;
    constexpr float kCoffeesLaunchSpread = 1.2f;
    constexpr float kCoffeesSpatialCellSize = 0.7f;
    constexpr float kCoffeesCanTopY = 0.85f;
    constexpr float kCoffeesCanTopRadius = 0.7f;
    constexpr float kCoffeesCanWallDamping = 0.45f;
    constexpr float kCoffeesAngularDamping = 0.96f;
    constexpr float kCoffeesRollingFollow = 0.18f;
    constexpr float kCoffeesTiltFollow = 0.08f;
    constexpr float kCoffeesMaxTilt = 1.15f;
    constexpr float kCoffeesWallBounceDamping = 0.75f;
    constexpr float kCoffeesStopSpeed = 0.08f;
    constexpr float kCoffeesStopPushPower = 0.015f;
    constexpr float kCoffeesPeerPushPower = 1.75f;

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

    bool IsVisibleFromCamera(const Camera* camera, const Vector3& position, float radius) {
        if (camera == nullptr) {
            return false;
        }

        const Vector3 viewPosition = Function::TransformVM(position, camera->GetViewMatrix());
        const float nearZ = camera->GetNearZ();
        const float farZ = camera->GetFarZ();

        if (viewPosition.z + radius < nearZ || viewPosition.z - radius > farZ) {
            return false;
        }

        const float halfFovY = camera->GetFovY() * 0.5f;
        const float halfHeight = std::tan(halfFovY) * viewPosition.z;
        const float halfWidth = halfHeight * camera->GetAspectRatio();

        if (std::abs(viewPosition.x) > halfWidth + radius) {
            return false;
        }
        if (std::abs(viewPosition.y) > halfHeight + radius) {
            return false;
        }

        return true;
    }

    CellCoord ComputeCellCoord(const Vector3& position) {
        return {
            static_cast<int32_t>(std::floor(position.x / kCoffeesSpatialCellSize)),
            static_cast<int32_t>(std::floor(position.y / kCoffeesSpatialCellSize)),
            static_cast<int32_t>(std::floor(position.z / kCoffeesSpatialCellSize)),
        };
    }
} // namespace

Coffees::Coffees() { instancedObject_ = std::make_unique<InstancedObject3d>(); }
Coffees::~Coffees(){

}
void Coffees::Initialize() {
    ModelManager::GetInstance()->LoadModel(kCoffeesModelDirectory, kCoffeesModelName);
    instancedObject_->Initialize(kCoffeesModelName);
    instancedObject_->SetShininess(80.0f);
    instancedObject_->SetSpawnOrigin({ 0.0f, 0.0f, 0.0f });
    renderedInstanceCapacity_ = std::min(kCoffeesInitialVisibleCount, kCoffeesInstanceCount);
    instancedObject_->SetInstanceCount(renderedInstanceCapacity_);

    instances_.resize(kCoffeesInstanceCount);
    activeInstanceCount_ = 0;
    spawnTimer_ = 0.0f;
    nextSpawnInterval_ = kCoffeesMaxSpawnInterval;

    for (uint32_t i = 0; i < kCoffeesInstanceCount; ++i) {
        const float scale = kCoffeesMinScale + static_cast<float>(i % 4u) * kCoffeesScaleStep;
        instancedObject_->SetInstanceScale(i, { scale, scale, scale });

        instances_[i].position = spawnOrigin_;
        instances_[i].scale = scale;
        instances_[i].radius = 0.12f + scale * 0.22f;
        instances_[i].halfHeight = 0.18f + scale * 0.35f;
        instances_[i].velocity = { 0.0f, 0.0f, 0.0f };
        instances_[i].rotation = { 0.0f, 0.0f, 0.0f };
        instances_[i].angularVelocity = { 0.0f, 0.0f, 0.0f };
        instances_[i].isActive = false;
        instancedObject_->SetInstanceRotate(i, { 0.0f, 0.0f, 0.0f });
        instancedObject_->SetInstanceOffset(i, { 0.0f, -1000.0f, 0.0f });
    }

    simulationParams_.deltaTime = 1.0f / 60.0f;
    simulationParams_.floorY = 0.0f;
    simulationParams_.canTopCenter = { 0.0f, 0.0f, 0.0f };
    simulationParams_.canTopY = kCoffeesCanTopY;
    simulationParams_.canTopRadius = kCoffeesCanTopRadius;
    simulationParams_.gravity = kCoffeesGravity;
    simulationParams_.bounceDamping = kCoffeesBounceDamping;
    simulationParams_.separationBias = kCoffeesSeparationBias;
    simulationParams_.roomMinX = -6.9f;
    simulationParams_.roomMaxX = 6.9f;
    simulationParams_.roomMinZ = -6.9f;
    simulationParams_.roomMaxZ = 6.9f;
}

void Coffees::RunSimulation() {
    if (instances_.empty()) {
        return;
    }

    //衝突したかどうか
    bool hasCollidedThisFrame = false;

    const float deltaTime = simulationParams_.deltaTime;
    const float gravity = simulationParams_.gravity;
    const Vector3 canTopCenter = simulationParams_.canTopCenter;
    const float canTopRadius = simulationParams_.canTopRadius;
    const float floorY = simulationParams_.floorY;
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
        nextSpawnInterval_ = kCoffeesMaxSpawnInterval + (kCoffeesMinSpawnInterval - kCoffeesMaxSpawnInterval) * t;

        auto& spawnInstance = instances_[spawnIndex];
        const float seed = static_cast<float>(spawnIndex) * 1.6180339f;
        const float angle = seed * 6.2831853f;
        const float radial = std::fmod(seed * 0.73f, 1.0f) * kCoffeesSpawnAreaRadius;
        spawnInstance.position.x = spawnOrigin_.x + std::cos(angle) * radial;
        spawnInstance.position.z = spawnOrigin_.z + std::sin(angle) * radial;
        spawnInstance.position.y = spawnOrigin_.y;
        const Vector3 right = { -launchDirection_.z, 0.0f, launchDirection_.x };
        const float spreadRatio = std::fmod(seed * 0.37f, 1.0f) * 2.0f - 1.0f;
        spawnInstance.velocity = {
            launchDirection_.x * kCoffeesLaunchSpeed + right.x * spreadRatio * kCoffeesLaunchSpread,
            kCoffeesLaunchVerticalSpeed + std::abs(spreadRatio) * 0.25f,
            launchDirection_.z * kCoffeesLaunchSpeed + right.z * spreadRatio * kCoffeesLaunchSpread,
        };
        spawnInstance.rotation = { 0.0f, angle, 0.0f };
        spawnInstance.angularVelocity = { 0.0f, 0.0f, 0.0f };
        spawnInstance.isActive = true;
        ++activeInstanceCount_;
        EnsureInstanceCapacity(activeInstanceCount_);
        instancedObject_->SetInstanceRotate(spawnIndex, spawnInstance.rotation);
        instancedObject_->SetInstanceOffset(spawnIndex, spawnInstance.position);
    }

    static std::vector<Vector3> pendingPush;
    static std::vector<CellCoord> cellCoords;
    static std::vector<CellEntry> cellEntries;
    static std::vector<CellRange> cellRanges;

    pendingPush.resize(activeInstanceCount_);
    std::fill(pendingPush.begin(), pendingPush.end(), Vector3{ 0.0f, 0.0f, 0.0f });
    cellCoords.resize(activeInstanceCount_);
    cellEntries.resize(activeInstanceCount_);

    for (size_t i = 0; i < activeInstanceCount_; ++i) {
        auto& instance = instances_[i];
        instance.velocity.y += gravity * deltaTime;
        instance.position.x += instance.velocity.x * deltaTime;
        instance.position.y += instance.velocity.y * deltaTime;
        instance.position.z += instance.velocity.z * deltaTime;
        instance.angularVelocity.x *= kCoffeesAngularDamping;
        instance.angularVelocity.y *= kCoffeesAngularDamping;
        instance.angularVelocity.z *= kCoffeesAngularDamping;

        const float minCenterY = floorY + instance.halfHeight;
        const bool isGrounded = instance.position.y <= minCenterY;
        if (instance.position.y <= minCenterY) {
            instance.position.y = minCenterY;
            instance.velocity.y = 0.0f;
            instance.velocity.x *= kCoffeesGroundFriction;
            instance.velocity.z *= kCoffeesGroundFriction;
        }

        const float horizontalSpeedSq = instance.velocity.x * instance.velocity.x + instance.velocity.z * instance.velocity.z;
        if (horizontalSpeedSq < kCoffeesStaticFrictionSpeed * kCoffeesStaticFrictionSpeed && instance.velocity.y == 0.0f) {
            instance.velocity.x = 0.0f;
            instance.velocity.z = 0.0f;
        }
        if (isGrounded) {
            const float rollingRadius = std::max(instance.radius, 0.05f);
            const Vector3 targetAngularVelocity = { instance.velocity.z / rollingRadius, 0.0f, -instance.velocity.x / rollingRadius };
            instance.angularVelocity.x += (targetAngularVelocity.x - instance.angularVelocity.x) * kCoffeesRollingFollow;
            instance.angularVelocity.z += (targetAngularVelocity.z - instance.angularVelocity.z) * kCoffeesRollingFollow;

            const Vector3 desiredTilt = {
                std::clamp(-instance.velocity.z * 0.08f, -kCoffeesMaxTilt, kCoffeesMaxTilt),
                instance.rotation.y,
                std::clamp(instance.velocity.x * 0.08f, -kCoffeesMaxTilt, kCoffeesMaxTilt),
            };
            instance.rotation.x += (desiredTilt.x - instance.rotation.x) * kCoffeesTiltFollow;
            instance.rotation.z += (desiredTilt.z - instance.rotation.z) * kCoffeesTiltFollow;
        }

        if (canTopRadius > 0.0f) {
            const float deltaCenterX = instance.position.x - canTopCenter.x;
            const float deltaCenterZ = instance.position.z - canTopCenter.z;
            const float maxCanRadius = std::max(0.0f, canTopRadius - instance.radius);
            const float distanceSq = deltaCenterX * deltaCenterX + deltaCenterZ * deltaCenterZ;
            if (distanceSq > maxCanRadius * maxCanRadius && distanceSq > 1e-7f) {
                const float distance = std::sqrt(distanceSq);
                const float invDistance = 1.0f / distance;
                const Vector3 normal = { deltaCenterX * invDistance, 0.0f, deltaCenterZ * invDistance };
                instance.position.x = canTopCenter.x + normal.x * maxCanRadius;
                instance.position.z = canTopCenter.z + normal.z * maxCanRadius;

                const float velocityAlongNormal = instance.velocity.x * normal.x + instance.velocity.z * normal.z;
                if (velocityAlongNormal > 0.0f) {
                    instance.velocity.x -= (1.0f + kCoffeesCanWallDamping) * velocityAlongNormal * normal.x;
                    instance.velocity.z -= (1.0f + kCoffeesCanWallDamping) * velocityAlongNormal * normal.z;
                    const float tangentialSpin = (instance.velocity.x * -normal.z + instance.velocity.z * normal.x) * 0.15f;
                    instance.angularVelocity.y += tangentialSpin;
                }
            }
        }

        instance.rotation.x += instance.angularVelocity.x * deltaTime;
        instance.rotation.y += instance.angularVelocity.y * deltaTime;
        instance.rotation.z += instance.angularVelocity.z * deltaTime;
    }

    for (uint32_t i = 0; i < activeInstanceCount_; ++i) {
        const CellCoord coord = ComputeCellCoord(instances_[i].position);
        cellCoords[i] = coord;
        cellEntries[i] = { HashCell(coord.x, coord.y, coord.z), i };
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
        cellRanges.push_back({ key, begin, end });
        begin = end;
    }

    const auto findCellRange = [](const std::vector<CellRange>& ranges, int64_t key) -> const CellRange* {
        auto it = std::lower_bound(ranges.begin(), ranges.end(), key, [](const CellRange& range, int64_t value) { return range.key < value; });
        if (it == ranges.end() || it->key != key) {
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
                        const float deltaX = instances_[i].position.x - instances_[j].position.x;
                        const float deltaZ = instances_[i].position.z - instances_[j].position.z;
                        const float distSqXZ = deltaX * deltaX + deltaZ * deltaZ;
                        const float minDistSq = minDist * minDist;
                        const float deltaY = std::abs(instances_[i].position.y - instances_[j].position.y);
                        const float minHeight = instances_[i].halfHeight + instances_[j].halfHeight;

                        if (distSqXZ < minDistSq && distSqXZ > 1e-7f && deltaY < minHeight) {
                            const uint32_t upperIndex = instances_[i].position.y >= instances_[j].position.y ? static_cast<uint32_t>(i) : j;
                            const uint32_t lowerIndex = upperIndex == static_cast<uint32_t>(i) ? j : static_cast<uint32_t>(i);
                            const float upperBottom = instances_[upperIndex].position.y - instances_[upperIndex].halfHeight;
                            const float lowerTop = instances_[lowerIndex].position.y + instances_[lowerIndex].halfHeight;
                            if (upperBottom < lowerTop) {
                                instances_[upperIndex].velocity.y = 0.0f;
                            }
                            const float dist = std::sqrt(distSqXZ);
                            const float overlap = minDist - dist;
                            const float scale = (overlap * 0.5f + separationBias) / dist;
                            const Vector3 normal = { deltaX / dist, 0.0f, deltaZ / dist };
                            const Vector3 push = { deltaX * scale * kCoffeesPeerPushPower, 0.0f, deltaZ * scale * kCoffeesPeerPushPower };
                            pendingPush[i].x += push.x;
                            pendingPush[i].z += push.z;
                            pendingPush[j].x -= push.x;
                            pendingPush[j].z -= push.z;

                            const Vector3 relativeVelocity = {
                                instances_[i].velocity.x - instances_[j].velocity.x,
                                0.0f,
                                instances_[i].velocity.z - instances_[j].velocity.z,
                            };
                            const float relativeAlongNormal = relativeVelocity.x * normal.x + relativeVelocity.z * normal.z;

                            // 強く衝突した
                            if (relativeAlongNormal < -0.5f) {
                                hasCollidedThisFrame = true;
                                // 物理計算へ...
                            }

                            if (relativeAlongNormal < 0.0f) {
                                const float impulse = -(1.0f + kCoffeesCollisionDamping) * relativeAlongNormal * 0.5f * kCoffeesPeerPushPower;
                                const Vector3 impulseVec = { normal.x * impulse, 0.0f, normal.z * impulse };
                                instances_[i].velocity.x += impulseVec.x;
                                instances_[i].velocity.z += impulseVec.z;
                                instances_[j].velocity.x -= impulseVec.x;
                                instances_[j].velocity.z -= impulseVec.z;

                                const Vector3 relativeAfter = {
                                    instances_[i].velocity.x - instances_[j].velocity.x,
                                    0.0f,
                                    instances_[i].velocity.z - instances_[j].velocity.z,
                                };
                                const float relNormalAfter = relativeAfter.x * normal.x + relativeAfter.z * normal.z;
                                const Vector3 tangent = {
                                    relativeAfter.x - normal.x * relNormalAfter,
                                    0.0f,
                                    relativeAfter.z - normal.z * relNormalAfter,
                                };
                                instances_[i].velocity.x -= tangent.x * (1.0f - kCoffeesCollisionTangentialDamping) * 0.5f;
                                instances_[i].velocity.z -= tangent.z * (1.0f - kCoffeesCollisionTangentialDamping) * 0.5f;
                                instances_[j].velocity.x += tangent.x * (1.0f - kCoffeesCollisionTangentialDamping) * 0.5f;
                                instances_[j].velocity.z += tangent.z * (1.0f - kCoffeesCollisionTangentialDamping) * 0.5f;
                            }
                        }
                    }
                }
            }
        }
    }

    if (hasCollidedThisFrame && rand() % 4 == 0) {
        // 鳴らす
        SEManager::SoundPlay(SEManager::CAN, false);
    }


    for (size_t i = 0; i < activeInstanceCount_; ++i) {
        auto& instance = instances_[i];
        const float pushPowerSq = pendingPush[i].x * pendingPush[i].x + pendingPush[i].z * pendingPush[i].z;

        instance.position.x += pendingPush[i].x;
        instance.position.y += pendingPush[i].y;
        instance.position.z += pendingPush[i].z;

        const float minY = floorY + instance.halfHeight;
        if (instance.position.y < minY) {
            instance.position.y = minY;
            instance.velocity.y = 0.0f;
        }

        const float minX = roomMinX + instance.radius;
        const float maxX = roomMaxX - instance.radius;
        const float minZ = roomMinZ + instance.radius;
        const float maxZ = roomMaxZ - instance.radius;

        if (instance.position.x < minX) {
            instance.position.x = minX;
            if (instance.velocity.x < 0.0f) {
                instance.velocity.x *= -kCoffeesWallBounceDamping;
            }
        } else if (instance.position.x > maxX) {
            instance.position.x = maxX;
            if (instance.velocity.x > 0.0f) {
                instance.velocity.x *= -kCoffeesWallBounceDamping;
            }
        }

        if (instance.position.z < minZ) {
            instance.position.z = minZ;
            if (instance.velocity.z < 0.0f) {
                instance.velocity.z *= -kCoffeesWallBounceDamping;
            }
        } else if (instance.position.z > maxZ) {
            instance.position.z = maxZ;
            if (instance.velocity.z > 0.0f) {
                instance.velocity.z *= -kCoffeesWallBounceDamping;
            }
        }

        const float horizontalSpeedSq = instance.velocity.x * instance.velocity.x + instance.velocity.z * instance.velocity.z;
        const bool isGrounded = instance.position.y <= (minY + 0.0001f);
        if (isGrounded && horizontalSpeedSq < (kCoffeesStopSpeed * kCoffeesStopSpeed) && pushPowerSq < (kCoffeesStopPushPower * kCoffeesStopPushPower)) {
            instance.velocity.x = 0.0f;
            instance.velocity.z = 0.0f;
            instance.angularVelocity.x = 0.0f;
            instance.angularVelocity.z = 0.0f;
        }
        instancedObject_->SetInstanceOffset(i, instance.position);
    }

}
void Coffees::EnsureInstanceCapacity(uint32_t requiredCount) {
    if (requiredCount <= renderedInstanceCapacity_) {
        return;
    }

    uint32_t newCapacity = std::max(requiredCount, std::min(kCoffeesInstanceCount, std::max(renderedInstanceCapacity_ * 2u, 1u)));
    newCapacity = std::min(newCapacity, kCoffeesInstanceCount);
    const uint32_t previousCapacity = renderedInstanceCapacity_;
    renderedInstanceCapacity_ = newCapacity;
    instancedObject_->SetInstanceCount(renderedInstanceCapacity_);

    for (uint32_t i = 0; i < renderedInstanceCapacity_; ++i) {
        const float scale = instances_[i].scale;
        instancedObject_->SetInstanceScale(i, { scale, scale, scale });
        if (i < activeInstanceCount_ && instances_[i].isActive) {
            instancedObject_->SetInstanceOffset(i, instances_[i].position);
        } else if (i >= previousCapacity) {
            instancedObject_->SetInstanceOffset(i, { 0.0f, -1000.0f, 0.0f });
        }
    }
}
void Coffees::Update(Camera* camera, const Vector3& lightDirection) {
    simulationParams_.deltaTime = std::max(Object3dCommon::GetInstance()->GetDxCommon()->GetDeltaTime(), 1.0f / 120.0f);
    if (isSpilling_) {
        RunSimulation();
    }

    for (uint32_t i = 0; i < renderedInstanceCapacity_; ++i) {
        if (isSpilling_ && i < activeInstanceCount_ && instances_[i].isActive && IsVisibleFromCamera(camera, instances_[i].position, instances_[i].radius)) {
            instancedObject_->SetInstanceOffset(i, instances_[i].position);
        } else {
            instancedObject_->SetInstanceOffset(i, { 0.0f, -1000.0f, 0.0f });
        }
    }

    instancedObject_->Update(camera, lightDirection);
}

void Coffees::Draw() {
    if (!isSpilling_) {
        return;
    }
    instancedObject_->Draw();
}

uint32_t Coffees::GetInstanceCount() const { return activeInstanceCount_; }

void Coffees::SetSpawnOrigin(const Vector3& spawnOrigin) {
    spawnOrigin_ = spawnOrigin;
    simulationParams_.canTopCenter = { spawnOrigin.x, 0.0f, spawnOrigin.z };
}
void Coffees::SetLaunchDirection(const Vector3& launchDirection) {
    Vector3 horizontalDirection = { launchDirection.x, 0.0f, launchDirection.z };
    const float lengthSq = horizontalDirection.x * horizontalDirection.x + horizontalDirection.z * horizontalDirection.z;
    if (lengthSq <= 1e-6f) {
        launchDirection_ = { 0.0f, 0.0f, 1.0f };
        return;
    }

    const float invLength = 1.0f / std::sqrt(lengthSq);
    launchDirection_ = { horizontalDirection.x * invLength, 0.0f, horizontalDirection.z * invLength };
}
void Coffees::SetRoomBounds(float minX, float maxX, float minZ, float maxZ) {
    simulationParams_.roomMinX = std::min(minX, maxX);
    simulationParams_.roomMaxX = std::max(minX, maxX);
    simulationParams_.roomMinZ = std::min(minZ, maxZ);
    simulationParams_.roomMaxZ = std::max(minZ, maxZ);
}
void Coffees::SetFloorY(float floorY) { simulationParams_.floorY = floorY; }

void Coffees::SetSpawnContainment(const Vector3& center, float topY, float radius) {
    simulationParams_.canTopCenter = center;
    simulationParams_.canTopY = topY;
    simulationParams_.canTopRadius = radius;
}

void Coffees::StartSpill() { isSpilling_ = true; }

bool Coffees::CheckHitPlayer(const Vector3& playerPosition, float playerRadius, float minHitSpeed) const {
    if (!isSpilling_) {
        return false;
    }

    const float minHitSpeedSq = minHitSpeed * minHitSpeed;
    for (uint32_t i = 0; i < activeInstanceCount_; ++i) {
        const InstanceData& instance = instances_[i];
        if (!instance.isActive) {
            continue;
        }

        const float speedSq = instance.velocity.x * instance.velocity.x + instance.velocity.y * instance.velocity.y + instance.velocity.z * instance.velocity.z;
        if (speedSq < minHitSpeedSq) {
            continue;
        }

        const float dx = instance.position.x - playerPosition.x;
        const float dz = instance.position.z - playerPosition.z;
        const float collisionRadius = instance.radius + playerRadius;
        if (dx * dx + dz * dz <= collisionRadius * collisionRadius) {
            const float topA = instance.position.y + instance.halfHeight;
            const float bottomA = instance.position.y - instance.halfHeight;
            const float playerHalfHeight = 0.9f;
            const float topB = playerPosition.y + playerHalfHeight;
            const float bottomB = playerPosition.y - playerHalfHeight;
            if (bottomA <= topB && topA >= bottomB) {
                return true;
            }
        }
    }

    return false;
}