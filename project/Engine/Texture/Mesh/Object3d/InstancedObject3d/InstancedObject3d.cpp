#include "InstancedObject3d.h"
#include "Function.h"
#include "Object3d/Object3dCommon.h"
#include "DirectXCommon.h"
#include "TextureManager.h"
#include "SrvManager/SrvManager.h"
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

void InstancedObject3d::CreateMesh() {
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

void InstancedObject3d::CreateInstancingPipeline() {
	auto* dxCommon = Object3dCommon::GetInstance()->GetDxCommon();
	auto* device = dxCommon->GetDevice();
	const std::wstring vsPath = L"Resources/shader/Object3d/VS_Shader/InstancedObject3d.VS.hlsl";
	const std::wstring psPath = L"Resources/shader/Object3d/PS_Shader/InstancedObject3d.PS.hlsl";

	D3D12_DESCRIPTOR_RANGE descriptorRange{};
	descriptorRange.BaseShaderRegister = 13;
	descriptorRange.NumDescriptors = 1;
	descriptorRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	descriptorRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	D3D12_ROOT_PARAMETER rootParameters[2]{};
	rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
	rootParameters[0].DescriptorTable.NumDescriptorRanges = 1;
	rootParameters[0].DescriptorTable.pDescriptorRanges = &descriptorRange;
	rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	rootParameters[1].Descriptor.ShaderRegister = 13;

	D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc{};
	rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
	rootSignatureDesc.NumParameters = _countof(rootParameters);
	rootSignatureDesc.pParameters = rootParameters;

	Microsoft::WRL::ComPtr<ID3DBlob> signatureBlob;
	Microsoft::WRL::ComPtr<ID3DBlob> errorBlob;
	HRESULT hr = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);
	assert(SUCCEEDED(hr));
	hr = device->CreateRootSignature(0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignature_));
	assert(SUCCEEDED(hr));

	auto vsBlob = dxCommon->CompileShader(vsPath.c_str(), L"vs_6_0");
	auto psBlob = dxCommon->CompileShader(psPath.c_str(), L"ps_6_0");

	D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
	    {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
	    {"NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
	};

	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc{};
	psoDesc.pRootSignature = rootSignature_.Get();
	psoDesc.InputLayout = {inputLayout, _countof(inputLayout)};
	psoDesc.VS = {vsBlob->GetBufferPointer(), vsBlob->GetBufferSize()};
	psoDesc.PS = {psBlob->GetBufferPointer(), psBlob->GetBufferSize()};
	psoDesc.BlendState.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
	psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;
	psoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
	psoDesc.RasterizerState.DepthClipEnable = TRUE;
	psoDesc.DepthStencilState.DepthEnable = true;
	psoDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
	psoDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	psoDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	psoDesc.SampleDesc.Count = 1;
	hr = device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pipelineState_));
	assert(SUCCEEDED(hr));
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

	instanceSrvIndex_ = srvManager->Allocate();
	srvManager->CreateSRVforStructuredBuffer(instanceSrvIndex_, instanceResource_.Get(), kCoffeeInstanceCount_, sizeof(Matrix4x4));

	sceneConstantResource_ = object3dCommon->CreateBufferResource(sizeof(CoffeeSceneConstants));
	sceneConstantResource_->Map(0, nullptr, reinterpret_cast<void**>(&sceneConstants_));
	sceneConstants_->viewProjection = Function::MakeIdentity4x4();
	sceneConstants_->lightDirection = {0.1f, -0.5f, -0.2f, 1.0f};
}

void InstancedObject3d::Initialize() {
	CreateMesh();
	CreateInstancingPipeline();
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
	commandList->SetGraphicsRootSignature(rootSignature_.Get());
	commandList->SetPipelineState(pipelineState_.Get());
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	commandList->IASetVertexBuffers(0, 1, &vertexBufferView_);
	commandList->IASetIndexBuffer(&indexBufferView_);
	srvManager->SetGraphicsRootDescriptorTable(0, instanceSrvIndex_);
	commandList->SetGraphicsRootConstantBufferView(1, sceneConstantResource_->GetGPUVirtualAddress());
	commandList->DrawIndexedInstanced(static_cast<UINT>(coffeeIndices_.size()), kCoffeeInstanceCount_, 0, 0, 0);
}