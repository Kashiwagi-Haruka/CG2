#define NOMINMAX
#include "Object3d/Object3dCommon.h"
#include "DirectXCommon.h"
#include "Logger.h"
#include "SrvManager/SrvManager.h"
#include "TextureManager.h"
#include <algorithm>
#include <cassert>
#include <cstring>

std::unique_ptr<Object3dCommon> Object3dCommon::instance = nullptr;

Object3dCommon::Object3dCommon() {}
Object3dCommon::~Object3dCommon() {}

Object3dCommon* Object3dCommon::GetInstance() {

	if (instance == nullptr) {
		instance = std::make_unique<Object3dCommon>();
	}
	return instance.get();
}

void Object3dCommon::Finalize() { instance.reset(); }

void Object3dCommon::Initialize(DirectXCommon* dxCommon) {
	dxCommon_ = dxCommon;
	pso_ = std::make_unique<CreatePSO>(dxCommon_);
	pso_->Create(D3D12_CULL_MODE_BACK);
	psoNoDepth_ = std::make_unique<CreatePSO>(dxCommon_);
	psoNoDepth_->Create(D3D12_CULL_MODE_BACK, false);

	SetEnvironmentMapTexture("Resources/3d/skydome.png");

	// Directional Light の共通バッファ作成
	directionalLightResource_ = CreateBufferResource(sizeof(DirectionalLight));
	assert(directionalLightResource_);
	directionalLightResource_->Map(0, nullptr, reinterpret_cast<void**>(&directionalLightData_));
	assert(directionalLightData_);

	*directionalLightData_ = {
	    {1.0f, 1.0f, 1.0f, 1.0f},
        {0.0f, -1.0f, 0.0f},
        1.0f
    };
	directionalLightResource_->Unmap(0, nullptr);
	pointLightResource_ = CreateBufferResource(sizeof(PointLight) * kMaxPointLights);
	assert(pointLightResource_);
	pointLightCountResource_ = CreateBufferResource(sizeof(PointLightCount));
	assert(pointLightCountResource_);

	auto* srvManager = TextureManager::GetInstance()->GetSrvManager();
	pointLightSrvIndex_ = srvManager->Allocate();
	srvManager->CreateSRVforStructuredBuffer(pointLightSrvIndex_, pointLightResource_.Get(), static_cast<UINT>(kMaxPointLights), sizeof(PointLight));

	spotLightResource_ = CreateBufferResource(sizeof(SpotLight) * kMaxSpotLights);
	assert(spotLightResource_);
	spotLightCountResource_ = CreateBufferResource(sizeof(SpotLightCount));
	assert(spotLightCountResource_);

	spotLightSrvIndex_ = srvManager->Allocate();
	srvManager->CreateSRVforStructuredBuffer(spotLightSrvIndex_, spotLightResource_.Get(), static_cast<UINT>(kMaxSpotLights), sizeof(SpotLight));

	areaLightResource_ = CreateBufferResource(sizeof(AreaLight) * kMaxAreaLights);
	assert(areaLightResource_);
	areaLightCountResource_ = CreateBufferResource(sizeof(AreaLightCount));
	assert(areaLightCountResource_);

	areaLightSrvIndex_ = srvManager->Allocate();
	srvManager->CreateSRVforStructuredBuffer(areaLightSrvIndex_, areaLightResource_.Get(), static_cast<UINT>(kMaxAreaLights), sizeof(AreaLight));

}
void Object3dCommon::SetEnvironmentMapTexture(const std::string& filePath) {
	environmentMapPath_ = filePath;
	TextureManager::GetInstance()->LoadTextureName(environmentMapPath_);
	environmentMapSrvIndex_ = TextureManager::GetInstance()->GetTextureIndexByfilePath(environmentMapPath_);
}
void Object3dCommon::DrawCommon() {

	dxCommon_->GetCommandList()->SetGraphicsRootSignature(pso_->GetRootSignature().Get());
	dxCommon_->GetCommandList()->SetPipelineState(pso_->GetGraphicsPipelineState(blendMode_).Get());

	dxCommon_->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}
void Object3dCommon::DrawCommonNoDepth() {
	dxCommon_->GetCommandList()->SetGraphicsRootSignature(psoNoDepth_->GetRootSignature().Get());
	dxCommon_->GetCommandList()->SetPipelineState(psoNoDepth_->GetGraphicsPipelineState(blendMode_).Get());

	dxCommon_->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}
void Object3dCommon::SetDirectionalLight(DirectionalLight& light) { *directionalLightData_ = light; }
void Object3dCommon::SetBlendMode(BlendMode blendMode) {
	blendMode_ = blendMode;
	dxCommon_->GetCommandList()->SetPipelineState(pso_->GetGraphicsPipelineState(blendMode_).Get());
}
void Object3dCommon::SetPointLights(const PointLight* pointLights, uint32_t count) {
	uint32_t clampedCount = std::min(count, static_cast<uint32_t>(kMaxPointLights));
	pointLightResource_->Map(0, nullptr, reinterpret_cast<void**>(&pointlightData_));
	std::memset(pointlightData_, 0, sizeof(PointLight) * kMaxPointLights);
	if (pointLights && clampedCount > 0) {
		std::memcpy(pointlightData_, pointLights, sizeof(PointLight) * clampedCount);
	}
	pointLightResource_->Unmap(0, nullptr);

	pointLightCountResource_->Map(0, nullptr, reinterpret_cast<void**>(&pointLightCountData_));
	pointLightCountData_->count = clampedCount;
	pointLightCountResource_->Unmap(0, nullptr);
}
void Object3dCommon::SetSpotLights(const SpotLight* spotLights, uint32_t count) {
	uint32_t clampedCount = std::min(count, static_cast<uint32_t>(kMaxSpotLights));
	spotLightResource_->Map(0, nullptr, reinterpret_cast<void**>(&spotLightData_));
	std::memset(spotLightData_, 0, sizeof(SpotLight) * kMaxSpotLights);
	if (spotLights && clampedCount > 0) {
		std::memcpy(spotLightData_, spotLights, sizeof(SpotLight) * clampedCount);
	}
	spotLightResource_->Unmap(0, nullptr);

	spotLightCountResource_->Map(0, nullptr, reinterpret_cast<void**>(&spotLightCountData_));
	spotLightCountData_->count = clampedCount;
	spotLightCountResource_->Unmap(0, nullptr);
}
void Object3dCommon::SetAreaLights(const AreaLight* areaLights, uint32_t count) {
	uint32_t clampedCount = std::min(count, static_cast<uint32_t>(kMaxAreaLights));
	areaLightResource_->Map(0, nullptr, reinterpret_cast<void**>(&areaLightData_));
	std::memset(areaLightData_, 0, sizeof(AreaLight) * kMaxAreaLights);
	if (areaLights && clampedCount > 0) {
		std::memcpy(areaLightData_, areaLights, sizeof(AreaLight) * clampedCount);
	}
	areaLightResource_->Unmap(0, nullptr);

	areaLightCountResource_->Map(0, nullptr, reinterpret_cast<void**>(&areaLightCountData_));
	areaLightCountData_->count = clampedCount;
	areaLightCountResource_->Unmap(0, nullptr);
}
Microsoft::WRL::ComPtr<ID3D12Resource> Object3dCommon::CreateBufferResource(size_t sizeInBytes) {
	// バッファの設定(UPLOAD用に変更)
	D3D12_HEAP_PROPERTIES heapProperties = {};
	heapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;

	D3D12_RESOURCE_DESC resourceDesc = {};
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resourceDesc.Width = sizeInBytes;
	resourceDesc.Height = 1;
	resourceDesc.DepthOrArraySize = 1;
	resourceDesc.MipLevels = 1;
	resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
	resourceDesc.SampleDesc.Count = 1;
	resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

	Microsoft::WRL::ComPtr<ID3D12Resource> bufferResource = nullptr;

	HRESULT hr_ = dxCommon_->GetDevice()->CreateCommittedResource(
	    &heapProperties, D3D12_HEAP_FLAG_NONE, &resourceDesc,
	    D3D12_RESOURCE_STATE_GENERIC_READ, // Uploadならこれ
	    nullptr, IID_PPV_ARGS(&bufferResource));

	if (FAILED(hr_)) {
		return nullptr;
	}

	return bufferResource;
}