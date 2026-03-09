#include "InstancedObject3d.h"
#include "DirectXCommon.h"
#include "Function.h"
#include "Model/Model.h"
#include "Model/ModelManager.h"
#include "Object3d/Object3dCommon.h"
#include "SrvManager/SrvManager.h"
#include "TextureManager.h"
#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstring>
#include <numbers>
#include <random>

namespace {
constexpr uint32_t kCoffeeSlices = 10;
constexpr float kCoffeeRadius = 0.03f;
constexpr float kCoffeeHeight = 0.10f;
constexpr float kCoffeeTopY = kCoffeeHeight * 0.5f;
constexpr float kCoffeeBottomY = -kCoffeeHeight * 0.5f;
constexpr float kRoomWidth = 9.0f;
constexpr float kRoomDepth = 15.0f;
constexpr float kSpawnMargin = 0.08f;
constexpr float kCellSize = kCoffeeRadius * 2.1f;
constexpr float kCollisionRadius = kCoffeeRadius * 1.4f;
} // namespace

void InstancedObject3d::CreateDefaultMesh() {
	coffeeVertices_.clear();
	coffeeIndices_.clear();
	for (uint32_t i = 0; i < kCoffeeSlices; ++i) {
		const float t = (static_cast<float>(i) / static_cast<float>(kCoffeeSlices)) * (std::numbers::pi_v<float> * 2.0f);
		const float x = std::cos(t) * kCoffeeRadius;
		const float z = std::sin(t) * kCoffeeRadius;
		const Vector3 sideNormal = Function::Normalize({x, 0.0f, z});
		coffeeVertices_.push_back({
		    {x, kCoffeeTopY, z},
            sideNormal
        });
		coffeeVertices_.push_back({
		    {x, kCoffeeBottomY, z},
            sideNormal
        });
	}
	const uint32_t topCenterIndex = static_cast<uint32_t>(coffeeVertices_.size());
	coffeeVertices_.push_back({
	    {0.0f, kCoffeeTopY, 0.0f},
        {0.0f, 1.0f,        0.0f}
    });
	const uint32_t bottomCenterIndex = static_cast<uint32_t>(coffeeVertices_.size());
	coffeeVertices_.push_back({
	    {0.0f, kCoffeeBottomY, 0.0f},
        {0.0f, -1.0f,          0.0f}
    });

	for (uint32_t i = 0; i < kCoffeeSlices; ++i) {
		const uint32_t next = (i + 1) % kCoffeeSlices;
		const uint32_t top0 = i * 2;
		const uint32_t bottom0 = i * 2 + 1;
		const uint32_t top1 = next * 2;
		const uint32_t bottom1 = next * 2 + 1;
		coffeeIndices_.insert(coffeeIndices_.end(), {top0, top1, bottom0, bottom0, top1, bottom1, topCenterIndex, top0, top1, bottomCenterIndex, bottom1, bottom0});
	}
}

void InstancedObject3d::CreateMeshFromModel(const Model& model) {
	coffeeVertices_.clear();
	coffeeIndices_.clear();

	const auto& modelData = model.GetModelData();
	coffeeVertices_.reserve(modelData.vertices.size());
	for (const auto& vertex : modelData.vertices) {
		coffeeVertices_.push_back({
		    {vertex.position.x, vertex.position.y, vertex.position.z},
		    vertex.normal,
		});
	}

	coffeeIndices_ = modelData.indices;
	if (coffeeIndices_.empty()) {
		coffeeIndices_.resize(coffeeVertices_.size());
		for (uint32_t i = 0; i < coffeeIndices_.size(); ++i) {
			coffeeIndices_[i] = i;
		}
	}
}

void InstancedObject3d::CreateMesh() {
	if (modelPath_.empty()) {
		CreateDefaultMesh();
		return;
	}

	std::unique_ptr<Model> modelInstance = ModelManager::GetInstance()->CreateModelInstance(modelPath_);
	if (modelInstance) {
		CreateMeshFromModel(*modelInstance);
		return;
	}

	Model* model = ModelManager::GetInstance()->FindModel(modelPath_);
	if (model) {
		CreateMeshFromModel(*model);
		return;
	}

	CreateDefaultMesh();
}

void InstancedObject3d::CreateInstancingPipeline() {
	const std::wstring vsPath = L"Resources/shader/Object3d/VS_Shader/InstancedObject3d.VS.hlsl";
	const std::wstring psPath = L"Resources/shader/Object3d/PS_Shader/InstancedObject3d.PS.hlsl";
	pso_ = std::make_unique<CreatePSO>(Object3dCommon::GetInstance()->GetDxCommon());
	pso_->CreateInstancedObject3d(psPath, vsPath);
}
void InstancedObject3d::CreateBuffers() {
	auto* object3dCommon = Object3dCommon::GetInstance();
	auto* srvManager = TextureManager::GetInstance()->GetSrvManager();

	vertexResource_ = object3dCommon->CreateBufferResource(sizeof(CoffeeVertex) * coffeeVertices_.size());
	CoffeeVertex* mappedVertex = nullptr;
	vertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&mappedVertex));
	std::memcpy(mappedVertex, coffeeVertices_.data(), sizeof(CoffeeVertex) * coffeeVertices_.size());
	vertexBufferView_ = {vertexResource_->GetGPUVirtualAddress(), static_cast<UINT>(sizeof(CoffeeVertex) * coffeeVertices_.size()), sizeof(CoffeeVertex)};

	indexResource_ = object3dCommon->CreateBufferResource(sizeof(uint32_t) * coffeeIndices_.size());
	uint32_t* mappedIndex = nullptr;
	indexResource_->Map(0, nullptr, reinterpret_cast<void**>(&mappedIndex));
	std::memcpy(mappedIndex, coffeeIndices_.data(), sizeof(uint32_t) * coffeeIndices_.size());
	indexBufferView_.BufferLocation = indexResource_->GetGPUVirtualAddress();
	indexBufferView_.SizeInBytes = static_cast<UINT>(sizeof(uint32_t) * coffeeIndices_.size());
	indexBufferView_.Format = DXGI_FORMAT_R32_UINT;

	instanceResource_ = object3dCommon->CreateBufferResource(sizeof(Matrix4x4) * kCoffeeInstanceCount_);
	Matrix4x4* mappedInstance = nullptr;
	instanceResource_->Map(0, nullptr, reinterpret_cast<void**>(&mappedInstance));

	const float minX = -kRoomWidth * 0.5f + kSpawnMargin;
	const float maxX = kRoomWidth * 0.5f - kSpawnMargin;
	const float minZ = -kRoomDepth * 0.5f + kSpawnMargin;
	const float maxZ = kRoomDepth * 0.5f - kSpawnMargin;
	const float invCell = 1.0f / kCellSize;
	const int32_t gridWidth = static_cast<int32_t>((maxX - minX) * invCell) + 2;
	const int32_t gridHeight = static_cast<int32_t>((maxZ - minZ) * invCell) + 2;
	std::vector<std::vector<Vector3>> spatialGrid(static_cast<size_t>(gridWidth * gridHeight));
	auto cellIndex = [=](int32_t gx, int32_t gz) -> int32_t {
		gx = std::clamp(gx, 0, gridWidth - 1);
		gz = std::clamp(gz, 0, gridHeight - 1);
		return gz * gridWidth + gx;
	};

	std::mt19937 randomEngine(20250518u);
	std::uniform_real_distribution<float> posXDist(minX, maxX);
	std::uniform_real_distribution<float> posZDist(minZ, maxZ);
	std::uniform_real_distribution<float> rotYDist(0.0f, std::numbers::pi_v<float> * 2.0f);
	std::uniform_real_distribution<float> scaleDist(0.75f, 1.15f);

	for (uint32_t i = 0; i < kCoffeeInstanceCount_; ++i) {
		Vector3 position{};
		bool placed = false;
		for (uint32_t retry = 0; retry < 64 && !placed; ++retry) {
			position = {posXDist(randomEngine), kCollisionRadius, posZDist(randomEngine)};
			const int32_t gx = static_cast<int32_t>((position.x - minX) * invCell);
			const int32_t gz = static_cast<int32_t>((position.z - minZ) * invCell);
			placed = true;
			for (int32_t dz = -1; dz <= 1 && placed; ++dz) {
				for (int32_t dx = -1; dx <= 1 && placed; ++dx) {
					for (const Vector3& other : spatialGrid[static_cast<size_t>(cellIndex(gx + dx, gz + dz))]) {
						if (Function::LengthSquared(position - other) < (kCollisionRadius * 2.0f) * (kCollisionRadius * 2.0f)) {
							placed = false;
							break;
						}
					}
				}
			}
			if (placed) {
				spatialGrid[static_cast<size_t>(cellIndex(gx, gz))].push_back(position);
			}
		}
		if (!placed) {
			position = {posXDist(randomEngine), kCollisionRadius, posZDist(randomEngine)};
		}
		const float yaw = rotYDist(randomEngine);
		const float scale = scaleDist(randomEngine);
		mappedInstance[i] = Function::MakeAffineMatrix(Vector3{scale, scale, scale}, Vector3{0.0f, yaw, 0.0f}, position);
	}

		if (!hasInstanceSrvIndex_) {
		instanceSrvIndex_ = srvManager->Allocate();
		hasInstanceSrvIndex_ = true;
	}
	srvManager->CreateSRVforStructuredBuffer(instanceSrvIndex_, instanceResource_.Get(), kCoffeeInstanceCount_, sizeof(Matrix4x4));

	sceneConstantResource_ = object3dCommon->CreateBufferResource(sizeof(CoffeeSceneConstants));
	sceneConstantResource_->Map(0, nullptr, reinterpret_cast<void**>(&sceneConstants_));
	sceneConstants_->viewProjection = Function::MakeIdentity4x4();
	sceneConstants_->lightDirection = {0.1f, -0.5f, -0.2f, 1.0f};
}

void InstancedObject3d::Initialize(const std::string& modelPath) {
	modelPath_ = modelPath;
	CreateMesh();
	CreateInstancingPipeline();
	CreateBuffers();
	isInitialized_ = true;
}

void InstancedObject3d::SetModel(const std::string& modelPath) {
	modelPath_ = modelPath;
	if (!isInitialized_) {
		return;
	}
	CreateMesh();
	CreateBuffers();
}

void InstancedObject3d::Update(const Camera* camera, const Vector3& lightDirection) {
	sceneConstants_->viewProjection = camera->GetViewProjectionMatrix();
	sceneConstants_->lightDirection = {lightDirection.x, lightDirection.y, lightDirection.z, 1.0f};
}

void InstancedObject3d::Draw() {
	auto* dxCommon = Object3dCommon::GetInstance()->GetDxCommon();
	auto* commandList = dxCommon->GetCommandList();
	auto* srvManager = TextureManager::GetInstance()->GetSrvManager();
	ID3D12DescriptorHeap* descriptorHeaps[] = {srvManager->GetDescriptorHeap().Get()};
	commandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);
	commandList->SetGraphicsRootSignature(pso_->GetRootSignature().Get());
	commandList->SetPipelineState(pso_->GetGraphicsPipelineState(BlendMode::kBlendModeNone).Get());
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	commandList->IASetVertexBuffers(0, 1, &vertexBufferView_);
	commandList->IASetIndexBuffer(&indexBufferView_);
	srvManager->SetGraphicsRootDescriptorTable(0, instanceSrvIndex_);
	commandList->SetGraphicsRootConstantBufferView(1, sceneConstantResource_->GetGPUVirtualAddress());
	commandList->DrawIndexedInstanced(static_cast<UINT>(coffeeIndices_.size()), kCoffeeInstanceCount_, 0, 0, 0);
}