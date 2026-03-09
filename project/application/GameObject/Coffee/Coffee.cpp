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

namespace {
constexpr const char* kCoffeeModelDirectory = "Resources/TD3_3102/3d/Coffee";
constexpr const char* kCoffeeModelName = "Coffee";
constexpr float kCoffeeStartHeight = 3.0f;
constexpr float kCoffeeSpawnHeightJitter = 0.25f;
constexpr uint32_t kCoffeeInstanceCount = 1000;
constexpr uint32_t kCoffeeSpawnColumns = 4;
constexpr float kCoffeeSpawnSpacingX = 2.4f;
constexpr float kCoffeeSpawnSpacingZ = 2.8f;
constexpr float kCoffeeMinScale = 0.22f;
constexpr float kCoffeeScaleStep = 0.04f;
constexpr float kCoffeeGravity = -15.0f;
constexpr float kCoffeeBounceDamping = 0.8f;
constexpr float kCoffeeSeparationBias = 0.001f;

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

	const float columnOffset = (static_cast<float>(kCoffeeSpawnColumns) - 1.0f) * 0.5f;
	const uint32_t rowCount = (kCoffeeInstanceCount + kCoffeeSpawnColumns - 1u) / kCoffeeSpawnColumns;
	const float rowOffset = (static_cast<float>(rowCount) - 1.0f) * 0.5f;
	for (uint32_t i = 0; i < kCoffeeInstanceCount; ++i) {
		const uint32_t column = i % kCoffeeSpawnColumns;
		const uint32_t row = i / kCoffeeSpawnColumns;
		const float scale = kCoffeeMinScale + static_cast<float>(i % 4u) * kCoffeeScaleStep;
		instancedObject_->SetInstanceScale(i, {scale, scale, scale});

		const float x = (static_cast<float>(column) - columnOffset) * kCoffeeSpawnSpacingX;
		const float z = (static_cast<float>(row) - rowOffset) * kCoffeeSpawnSpacingZ;
		const float y = kCoffeeStartHeight + static_cast<float>((i * 5u) % 9u) * kCoffeeSpawnHeightJitter;
		instances_[i].position = {x, y, z};
		instances_[i].scale = scale;
		instances_[i].radius = 0.2f + scale * 0.35f;
		instancedObject_->SetInstanceOffset(i, instances_[i].position);
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

	std::vector<Vector2> pendingPush(instances_.size(), {0.0f, 0.0f});

	for (size_t i = 0; i < instances_.size(); ++i) {
		auto& instance = instances_[i];
		instance.velocityY += gravity * deltaTime;
		instance.position.y += instance.velocityY * deltaTime;
		if (instance.position.y <= floorY) {
			instance.position.y = floorY;
			instance.velocityY = -instance.velocityY * bounceDamping;
		}
	}

	for (size_t i = 0; i < instances_.size(); ++i) {
		for (size_t j = i + 1; j < instances_.size(); ++j) {
			const Vector2 posI = {instances_[i].position.x, instances_[i].position.z};
			const Vector2 posJ = {instances_[j].position.x, instances_[j].position.z};
			const float minDist = instances_[i].radius + instances_[j].radius;
			const Vector2 delta = {posI.x - posJ.x, posI.y - posJ.y};
			const float distSq = delta.x * delta.x + delta.y * delta.y;
			const float minDistSq = minDist * minDist;

			if (distSq < minDistSq && distSq > 1e-7f) {
				const float dist = std::sqrt(distSq);
				const float overlap = minDist - dist;
				const float scale = (overlap * 0.5f + separationBias) / dist;
				const Vector2 push = {delta.x * scale, delta.y * scale};
				pendingPush[i].x += push.x;
				pendingPush[i].y += push.y;
				pendingPush[j].x -= push.x;
				pendingPush[j].y -= push.y;
			}
		}
	}

	for (size_t i = 0; i < instances_.size(); ++i) {
		auto& instance = instances_[i];
		instance.position.x = std::clamp(instance.position.x + pendingPush[i].x, roomMinX, roomMaxX);
		instance.position.z = std::clamp(instance.position.z + pendingPush[i].y, roomMinZ, roomMaxZ);
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

uint32_t Coffee::GetInstanceCount() const { return instancedObject_->GetInstanceCount(); }