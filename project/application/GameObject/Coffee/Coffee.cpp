#define NOMINMAX
#include "Coffee.h"
#include "DirectXCommon.h"
#include "Model/ModelManager.h"
#include "Object3d/Object3dCommon.h"
#include "SrvManager/SrvManager.h"
#include "TextureManager.h"
#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstring>
#include <unordered_map>

namespace {
constexpr const char* kCoffeeModelDirectory = "Resources/TD3_3102/3d/Coffee";
constexpr const char* kCoffeeModelName = "Coffee";
constexpr uint32_t kCoffeeInstanceCount = 10000;
constexpr Vector3 kCoffeeSpawnOrigin = {0.0f, 5.0f, 0.0f};
constexpr float kCoffeeMinScale = 0.22f;
constexpr float kCoffeeScaleStep = 0.0f;
constexpr float kCoffeeGravity = -15.0f;
constexpr float kCoffeeBounceDamping = 0.0f;
constexpr float kCoffeeGroundFriction = 0.93f;
constexpr float kCoffeeCollisionDamping = 0.85f;
constexpr float kCoffeeSeparationBias = 0.001f;
constexpr float kCoffeeMinSpawnInterval = 0.035f;
constexpr float kCoffeeMaxSpawnInterval = 0.18f;
constexpr float kCoffeeSpawnAreaRadius = 0.35f;
constexpr float kCoffeeSpatialCellSize = 0.6f;

int64_t HashCell(int32_t x, int32_t y, int32_t z) { return (static_cast<int64_t>(x) << 42) ^ (static_cast<int64_t>(y) << 21) ^ static_cast<int64_t>(z); }

struct CellCoord {
	int32_t x;
	int32_t y;
	int32_t z;
};

CellCoord ComputeCellCoord(const Vector3& position) {
	return {
	    static_cast<int32_t>(std::floor(position.x / kCoffeeSpatialCellSize)),
	    static_cast<int32_t>(std::floor(position.y / kCoffeeSpatialCellSize)),
	    static_cast<int32_t>(std::floor(position.z / kCoffeeSpatialCellSize)),
	};
}
Microsoft::WRL::ComPtr<ID3D12Resource> CreateDefaultBufferResource(DirectXCommon* dxCommon, size_t sizeInBytes, D3D12_RESOURCE_FLAGS flags, D3D12_RESOURCE_STATES initialState) {
	D3D12_HEAP_PROPERTIES heapProperties{};
	heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;

	D3D12_RESOURCE_DESC resourceDesc{};
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resourceDesc.Width = sizeInBytes;
	resourceDesc.Height = 1;
	resourceDesc.DepthOrArraySize = 1;
	resourceDesc.MipLevels = 1;
	resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
	resourceDesc.SampleDesc.Count = 1;
	resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	resourceDesc.Flags = flags;

	Microsoft::WRL::ComPtr<ID3D12Resource> bufferResource;
	HRESULT hr = dxCommon->GetDevice()->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &resourceDesc, initialState, nullptr, IID_PPV_ARGS(&bufferResource));
	assert(SUCCEEDED(hr));
	return bufferResource;
}

Microsoft::WRL::ComPtr<ID3D12Resource> CreateReadbackBufferResource(DirectXCommon* dxCommon, size_t sizeInBytes) {
	D3D12_HEAP_PROPERTIES heapProperties{};
	heapProperties.Type = D3D12_HEAP_TYPE_READBACK;

	D3D12_RESOURCE_DESC resourceDesc{};
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resourceDesc.Width = sizeInBytes;
	resourceDesc.Height = 1;
	resourceDesc.DepthOrArraySize = 1;
	resourceDesc.MipLevels = 1;
	resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
	resourceDesc.SampleDesc.Count = 1;
	resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	Microsoft::WRL::ComPtr<ID3D12Resource> bufferResource;
	HRESULT hr = dxCommon->GetDevice()->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&bufferResource));
	assert(SUCCEEDED(hr));
	return bufferResource;
}
} // namespace

Coffee::Coffee() { instancedObject_ = std::make_unique<InstancedObject3d>(); }

void Coffee::Initialize() {
	ModelManager::GetInstance()->LoadModel(kCoffeeModelDirectory, kCoffeeModelName);
	instancedObject_->Initialize(kCoffeeModelName);
	instancedObject_->SetSpawnOrigin({0.0f, 0.0f, 0.0f});
	instancedObject_->SetInstanceCount(kCoffeeInstanceCount);

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

	InitializeSimulationResources();
}

void Coffee::InitializeSimulationResources() {
	auto* dxCommon = Object3dCommon::GetInstance()->GetDxCommon();
	auto* srvManager = TextureManager::GetInstance()->GetSrvManager();
	assert(dxCommon && srvManager);

	D3D12_DESCRIPTOR_RANGE descriptorRange{};
	descriptorRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
	descriptorRange.NumDescriptors = 1;
	descriptorRange.BaseShaderRegister = 0;
	descriptorRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	D3D12_ROOT_PARAMETER rootParameters[2]{};
	rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	rootParameters[0].DescriptorTable.NumDescriptorRanges = 1;
	rootParameters[0].DescriptorTable.pDescriptorRanges = &descriptorRange;
	rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	rootParameters[1].Descriptor.ShaderRegister = 0;

	D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc{};
	rootSignatureDesc.NumParameters = _countof(rootParameters);
	rootSignatureDesc.pParameters = rootParameters;

	Microsoft::WRL::ComPtr<ID3DBlob> signatureBlob;
	Microsoft::WRL::ComPtr<ID3DBlob> errorBlob;
	HRESULT hr = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);
	assert(SUCCEEDED(hr));
	hr = dxCommon->GetDevice()->CreateRootSignature(0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(&simulationRootSignature_));
	assert(SUCCEEDED(hr));

	auto csBlob = dxCommon->CompileShader(L"Resources/shader/Object3d/CS_Shader/InstancedObject3dCollision.CS.hlsl", L"cs_6_0");
	D3D12_COMPUTE_PIPELINE_STATE_DESC psoDesc{};
	psoDesc.pRootSignature = simulationRootSignature_.Get();
	psoDesc.CS = {csBlob->GetBufferPointer(), csBlob->GetBufferSize()};
	hr = dxCommon->GetDevice()->CreateComputePipelineState(&psoDesc, IID_PPV_ARGS(&simulationPipelineState_));
	assert(SUCCEEDED(hr));

	const size_t instanceDataSize = sizeof(InstanceData) * instances_.size();
	instanceDataResource_ = CreateDefaultBufferResource(dxCommon, instanceDataSize, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_DEST);
	instanceDataUploadResource_ = dxCommon->CreateBufferResource(instanceDataSize);
	instanceDataReadbackResource_ = CreateReadbackBufferResource(dxCommon, instanceDataSize);

	InstanceData* uploadData = nullptr;
	instanceDataUploadResource_->Map(0, nullptr, reinterpret_cast<void**>(&uploadData));
	std::memcpy(uploadData, instances_.data(), instanceDataSize);
	instanceDataUploadResource_->Unmap(0, nullptr);

	auto* commandList = dxCommon->GetCommandList();
	commandList->CopyResource(instanceDataResource_.Get(), instanceDataUploadResource_.Get());
	D3D12_RESOURCE_BARRIER initBarrier{};
	initBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	initBarrier.Transition.pResource = instanceDataResource_.Get();
	initBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
	initBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
	initBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	commandList->ResourceBarrier(1, &initBarrier);
	dxCommon->ExecuteCommandListAndWait();
	instanceDataState_ = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;

	instanceDataUavIndex_ = srvManager->Allocate();
	srvManager->CreateUAVforStructuredBuffer(instanceDataUavIndex_, instanceDataResource_.Get(), static_cast<UINT>(instances_.size()), sizeof(InstanceData));

	simulationParamsResource_ = dxCommon->CreateBufferResource(sizeof(SimulationParams));
	simulationParamsResource_->Map(0, nullptr, reinterpret_cast<void**>(&simulationParamsData_));
	simulationParamsData_->instanceCount = static_cast<uint32_t>(instances_.size());
	simulationParamsData_->deltaTime = 1.0f / 60.0f;
	simulationParamsData_->floorY = 0.0f;
	simulationParamsData_->gravity = kCoffeeGravity;
	simulationParamsData_->bounceDamping = kCoffeeBounceDamping;
	simulationParamsData_->separationBias = kCoffeeSeparationBias;
	simulationParamsData_->roomMinX = -100.0f;
	simulationParamsData_->roomMaxX = 100.0f;
	simulationParamsData_->roomMinZ = -800.0f;
	simulationParamsData_->roomMaxZ = 800.0f;
}

void Coffee::RunSimulation() {
	if (instances_.empty() || !simulationParamsData_) {
		return;
	}

	const float deltaTime = simulationParamsData_->deltaTime;
	const float gravity = simulationParamsData_->gravity;
	const float floorY = simulationParamsData_->floorY;
	const float bounceDamping = simulationParamsData_->bounceDamping;
	const float separationBias = simulationParamsData_->separationBias;
	const float roomMinX = simulationParamsData_->roomMinX;
	const float roomMaxX = simulationParamsData_->roomMaxX;
	const float roomMinZ = simulationParamsData_->roomMinZ;
	const float roomMaxZ = simulationParamsData_->roomMaxZ;

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
		instancedObject_->SetInstanceOffset(spawnIndex, spawnInstance.position);
		++activeInstanceCount_;
	}

	std::vector<Vector3> pendingPush(activeInstanceCount_, {0.0f, 0.0f, 0.0f});

	for (size_t i = 0; i < activeInstanceCount_; ++i) {
		auto& instance = instances_[i];
		instance.velocity.y += gravity * deltaTime;
		instance.position.x += instance.velocity.x * deltaTime;
		instance.position.y += instance.velocity.y * deltaTime;
		instance.position.z += instance.velocity.z * deltaTime;
		if (instance.position.y <= floorY) {
			instance.position.y = floorY;
			instance.velocity.y = 0.0f;
			instance.velocity.x *= kCoffeeGroundFriction;
			instance.velocity.z *= kCoffeeGroundFriction;
		}
	}

	std::unordered_map<int64_t, std::vector<uint32_t>> cellMap;
	cellMap.reserve(activeInstanceCount_);
	for (uint32_t i = 0; i < activeInstanceCount_; ++i) {
		const CellCoord coord = ComputeCellCoord(instances_[i].position);
		cellMap[HashCell(coord.x, coord.y, coord.z)].push_back(i);
	}

	for (size_t i = 0; i < activeInstanceCount_; ++i) {
		const CellCoord coord = ComputeCellCoord(instances_[i].position);
		for (int32_t dz = -1; dz <= 1; ++dz) {
			for (int32_t dy = -1; dy <= 1; ++dy) {
				for (int32_t dx = -1; dx <= 1; ++dx) {
					auto it = cellMap.find(HashCell(coord.x + dx, coord.y + dy, coord.z + dz));
					if (it == cellMap.end()) {
						continue;
					}
					for (const uint32_t j : it->second) {
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
		if (instance.position.y < floorY) {
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
void Coffee::Update(Camera* camera, const Vector3& lightDirection) {
	if (simulationParamsData_) {
		simulationParamsData_->deltaTime = std::max(Object3dCommon::GetInstance()->GetDxCommon()->GetDeltaTime(), 1.0f / 120.0f);
	}
	RunSimulation();
	instancedObject_->Update(camera, lightDirection);
}

void Coffee::Draw() { instancedObject_->Draw(); }

uint32_t Coffee::GetInstanceCount() const { return activeInstanceCount_; }
