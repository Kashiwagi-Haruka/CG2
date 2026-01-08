#include "Object3d/Object3dCommon.h"
#include "DirectXCommon.h"
#include "Logger.h"
#include <algorithm>
#include <cassert>

std::unique_ptr<Object3dCommon> Object3dCommon::instance = nullptr;

Object3dCommon::Object3dCommon() {}
Object3dCommon::~Object3dCommon() {}

Object3dCommon* Object3dCommon::GetInstance() {

	if (instance == nullptr) {
		instance = std::make_unique<Object3dCommon>();
	}
	return instance.get();
}

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
	pointLightResource_ = CreateBufferResource(sizeof(PointLightSet));
	assert(pointLightResource_);

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
	pointLightResource_->Map(0, nullptr, reinterpret_cast<void**>(&pointlightData_));
	const uint32_t maxPointLights = static_cast<uint32_t>(kMaxPointLights);
	uint32_t pointLightCount = std::min<uint32_t>(count, maxPointLights);
	pointlightData_->count = static_cast<int>(pointLightCount);
	for (uint32_t index = 0; index < pointLightCount; ++index) {
		pointlightData_->lights[index] = pointLights[index];
	}
	for (uint32_t index = pointLightCount; index < maxPointLights; ++index) {
		pointlightData_->lights[index] = {};
	}
	pointLightResource_->Unmap(0, nullptr);
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
