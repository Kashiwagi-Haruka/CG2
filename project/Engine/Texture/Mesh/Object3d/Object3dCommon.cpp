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

	spotLightResource_ = CreateBufferResource(sizeof(SpotLight));
	assert(spotLightResource_);
}

void Object3dCommon::DrawCommon() {

	dxCommon_->GetCommandList()->SetGraphicsRootSignature(pso_->GetRootSignature().Get());
	dxCommon_->GetCommandList()->SetPipelineState(pso_->GetGraphicsPipelineState(blendMode_).Get());

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
void Object3dCommon::SetSpotLight(SpotLight spotlight) {
	spotLightResource_->Map(0, nullptr, reinterpret_cast<void**>(&spotlightData_));
	spotlightData_->color = spotlight.color;
	spotlightData_->position = spotlight.position;
	spotlightData_->intensity = spotlight.intensity;
	spotlightData_->direction = spotlight.direction;
	spotlightData_->distance = spotlight.distance;
	spotlightData_->decay = spotlight.decay;
	spotlightData_->cosAngle = spotlight.cosAngle;
	spotlightData_->cosFalloffStart = spotlight.cosFalloffStart;
	spotLightResource_->Unmap(0, nullptr);
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