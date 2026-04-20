#define NOMINMAX
#include "Object3d/Object3dCommon.h"
#include "DirectXCommon.h"
#include "Function.h"
#include "Logger.h"
#include "SrvManager/SrvManager.h"
#include "TextureManager.h"
#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstring>

namespace {
struct ShadowRenderTarget {
	Microsoft::WRL::ComPtr<ID3D12Resource>* resource;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>* dsvHeap;
	uint32_t* srvIndex;
};

void CreateShadowRenderTarget(DirectXCommon* dxCommon, const D3D12_HEAP_PROPERTIES& heapProps, const D3D12_RESOURCE_DESC& shadowDesc, const D3D12_CLEAR_VALUE& clearValue, ShadowRenderTarget target) {
	HRESULT hr = dxCommon->GetDevice()->CreateCommittedResource(
	    &heapProps, D3D12_HEAP_FLAG_NONE, &shadowDesc, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, &clearValue, IID_PPV_ARGS(target.resource->ReleaseAndGetAddressOf()));
	assert(SUCCEEDED(hr));
	*target.dsvHeap = dxCommon->CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 1, false);
	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
	dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	dxCommon->GetDevice()->CreateDepthStencilView(target.resource->Get(), &dsvDesc, (*target.dsvHeap)->GetCPUDescriptorHandleForHeapStart());
	*target.srvIndex = TextureManager::GetInstance()->GetSrvManager()->Allocate();
	TextureManager::GetInstance()->GetSrvManager()->CreateSRVforTexture2D(*target.srvIndex, target.resource->Get(), DXGI_FORMAT_R32_FLOAT, 1);
}

void TransitionShadowResource(DirectXCommon* dxCommon, ID3D12Resource* resource, D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after) {
	if (!resource || before == after) {
		return;
	}
	D3D12_RESOURCE_BARRIER barrier{};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Transition.pResource = resource;
	barrier.Transition.StateBefore = before;
	barrier.Transition.StateAfter = after;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	dxCommon->GetCommandList()->ResourceBarrier(1, &barrier);
}
} // namespace
namespace {
enum ShadowMapPassType : int32_t {
	kShadowMapPassDirectional = 0,
	kShadowMapPassPoint = 1,
	kShadowMapPassSpot = 2,
	kShadowMapPassArea = 3,
};
Matrix4x4 MakeLightViewProjection(const Vector3& lightPosition, const Vector3& rawLightDirection, float nearPlane, float farPlane, float fovRadians) {
	Vector3 lightDirection = Function::Normalize(rawLightDirection);
	if (Function::Length(lightDirection) < 1.0e-5f) {
		lightDirection = {0.0f, -1.0f, 0.0f};
	}
	const Vector3 up = (std::abs(lightDirection.y) > 0.99f) ? Vector3{0.0f, 0.0f, 1.0f} : Vector3{0.0f, 1.0f, 0.0f};
	const Vector3 right = Function::Normalize(Function::Cross(up, lightDirection));
	const Vector3 cameraUp = Function::Cross(lightDirection, right);

	Matrix4x4 view = Function::MakeIdentity4x4();
	view.m[0][0] = right.x;
	view.m[1][0] = right.y;
	view.m[2][0] = right.z;
	view.m[0][1] = cameraUp.x;
	view.m[1][1] = cameraUp.y;
	view.m[2][1] = cameraUp.z;
	view.m[0][2] = lightDirection.x;
	view.m[1][2] = lightDirection.y;
	view.m[2][2] = lightDirection.z;
	view.m[3][0] = -Function::Dot(lightPosition, right);
	view.m[3][1] = -Function::Dot(lightPosition, cameraUp);
	view.m[3][2] = -Function::Dot(lightPosition, lightDirection);

	const float clampedNearPlane = std::max(0.01f, nearPlane);
	const float clampedFarPlane = std::max(clampedNearPlane + 0.1f, farPlane);
	const float clampedFov = std::clamp(fovRadians, 0.1f, Function::kPi - 0.1f);
	Matrix4x4 projection = Function::MakePerspectiveFovMatrix(clampedFov, 1.0f, clampedNearPlane, clampedFarPlane);
	return Function::Multiply(view, projection);
}
}
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

	psoToon_ = std::make_unique<CreatePSO>(dxCommon_);
	psoToon_->Create(D3D12_CULL_MODE_BACK, true, D3D12_FILL_MODE_SOLID, D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, L"Resources/shader/Object3d/PS_Shader/Object3dToon.PS.hlsl");

	psoEmissive_ = std::make_unique<CreatePSO>(dxCommon_);
	psoEmissive_->Create(D3D12_CULL_MODE_NONE, true, D3D12_FILL_MODE_SOLID, D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, L"Resources/shader/Object3d/PS_Shader/Object3dEmissive.PS.hlsl");

	psoNoCull_ = std::make_unique<CreatePSO>(dxCommon_);
	psoNoCull_->Create(D3D12_CULL_MODE_NONE);

	psoNoDepth_ = std::make_unique<CreatePSO>(dxCommon_);
	psoNoDepth_->Create(D3D12_CULL_MODE_BACK, false);

	psoNoCullDepth_ = std::make_unique<CreatePSO>(dxCommon_);
	psoNoCullDepth_->Create(D3D12_CULL_MODE_NONE, false);

	psoWireframe_ = std::make_unique<CreatePSO>(dxCommon_);
	psoWireframe_->Create(D3D12_CULL_MODE_NONE, true, D3D12_FILL_MODE_WIREFRAME);

	psoWireframeNoDepth_ = std::make_unique<CreatePSO>(dxCommon_);
	psoWireframeNoDepth_->Create(D3D12_CULL_MODE_NONE, false, D3D12_FILL_MODE_WIREFRAME);

	psoLine_ = std::make_unique<CreatePSO>(dxCommon_);
	psoLine_->Create(D3D12_CULL_MODE_NONE, true, D3D12_FILL_MODE_SOLID, D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE);

	psoLineNoDepth_ = std::make_unique<CreatePSO>(dxCommon_);
	psoLineNoDepth_->Create(D3D12_CULL_MODE_NONE, false, D3D12_FILL_MODE_SOLID, D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE);

	psoEditorGrid_ = std::make_unique<CreatePSO>(dxCommon_);
	psoEditorGrid_->Create(D3D12_CULL_MODE_NONE, true, D3D12_FILL_MODE_SOLID, D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, L"Resources/shader/Object3d/PS_Shader/Object3dGrid.PS.hlsl");

	psoSkinning_ = std::make_unique<CreatePSO>(dxCommon_, true);
	psoSkinning_->Create(D3D12_CULL_MODE_BACK);

	psoSkinningToon_ = std::make_unique<CreatePSO>(dxCommon_, true);
	psoSkinningToon_->Create(D3D12_CULL_MODE_BACK, true, D3D12_FILL_MODE_SOLID, D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, L"Resources/shader/Object3d/PS_Shader/Object3dToon.PS.hlsl");
	
	psoSkinningOutline_ = std::make_unique<CreatePSO>(dxCommon_, true);
	psoSkinningOutline_->Create(
	    D3D12_CULL_MODE_BACK, true, D3D12_FILL_MODE_SOLID, D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, L"Resources/shader/Object3d/PS_Shader/Object3dOutline.PS.hlsl",
	    L"Resources/shader/Object3d/VS_Shader/SkinningObject3dOutline.VS.hlsl");

	psoSkinningToonOutline_ = std::make_unique<CreatePSO>(dxCommon_, true);
	psoSkinningToonOutline_->Create(
	    D3D12_CULL_MODE_FRONT, true, D3D12_FILL_MODE_SOLID, D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, L"Resources/shader/Object3d/PS_Shader/SkinningObject3dToonOutline.PS.hlsl",
	    L"Resources/shader/Object3d/VS_Shader/SkinningObject3dToonOutline.VS.hlsl");

	SetEnvironmentMapTexture("Resources/3d/skydome.png");

	psoMirror_ = std::make_unique<CreatePSO>(dxCommon_);
	psoMirror_->Create(D3D12_CULL_MODE_BACK, true, D3D12_FILL_MODE_SOLID, D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, L"Resources/shader/Object3d/PS_Shader/Object3dMirror.PS.hlsl");

	psoOutline_ = std::make_unique<CreatePSO>(dxCommon_);
	psoOutline_->Create(
	    D3D12_CULL_MODE_BACK, true, D3D12_FILL_MODE_SOLID, D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, L"Resources/shader/Object3d/PS_Shader/Object3dOutline.PS.hlsl");

	psoPortal_ = std::make_unique<CreatePSO>(dxCommon_);
	psoPortal_->Create(
	    D3D12_CULL_MODE_BACK, true, D3D12_FILL_MODE_SOLID, D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, L"Resources/shader/Object3d/PS_Shader/Object3dPortal.PS.hlsl",
	    L"Resources/shader/Object3d/VS_Shader/Object3dPortal.VS.hlsl");

	psoShadow_ = std::make_unique<CreatePSO>(dxCommon_);
	psoShadow_->CreateShadow();

	// Directional Light の共通バッファ作成
	directionalLightResource_ = CreateBufferResource(sizeof(DirectionalCommonLight));
	assert(directionalLightResource_);
	directionalLightResource_->Map(0, nullptr, reinterpret_cast<void**>(&directionalLightData_));
	assert(directionalLightData_);

	*directionalLightData_ = {
	    {1.0f, 1.0f, 1.0f, 1.0f},
        {0.0f, -1.0f, 0.0f},
        1.0f, 1, {0.0f, 0.0f, 0.0f}
    };
	directionalLightResource_->Unmap(0, nullptr);
	pointLightResource_ = CreateBufferResource(sizeof(PointCommonLight) * kMaxPointLights);
	assert(pointLightResource_);
	pointLightCountResource_ = CreateBufferResource(sizeof(PointCommonLightCount));
	assert(pointLightCountResource_);

	pointLightSrvIndex_ = TextureManager::GetInstance()->GetSrvManager()->Allocate();
	TextureManager::GetInstance()->GetSrvManager()->CreateSRVforStructuredBuffer(pointLightSrvIndex_, pointLightResource_.Get(), static_cast<UINT>(kMaxPointLights), sizeof(PointCommonLight));

	spotLightResource_ = CreateBufferResource(sizeof(SpotCommonLight) * kMaxSpotLights);
	assert(spotLightResource_);
	spotLightCountResource_ = CreateBufferResource(sizeof(SpotCommonLightCount));
	assert(spotLightCountResource_);

	spotLightSrvIndex_ = TextureManager::GetInstance()->GetSrvManager()->Allocate();
	TextureManager::GetInstance()->GetSrvManager()->CreateSRVforStructuredBuffer(spotLightSrvIndex_, spotLightResource_.Get(), static_cast<UINT>(kMaxSpotLights), sizeof(SpotCommonLight));

	areaLightResource_ = CreateBufferResource(sizeof(AreaCommonLight) * kMaxAreaLights);
	assert(areaLightResource_);
	areaLightCountResource_ = CreateBufferResource(sizeof(AreaCommonLightCount));
	assert(areaLightCountResource_);

	areaLightSrvIndex_ = TextureManager::GetInstance()->GetSrvManager()->Allocate();
	TextureManager::GetInstance()->GetSrvManager()->CreateSRVforStructuredBuffer(areaLightSrvIndex_, areaLightResource_.Get(), static_cast<UINT>(kMaxAreaLights), sizeof(AreaCommonLight));
	
	shadowMapPassSettingsResource_ = CreateBufferResource(sizeof(ShadowMapPassSettings));
	assert(shadowMapPassSettingsResource_);
	shadowMapPassSettingsResource_->Map(0, nullptr, reinterpret_cast<void**>(&shadowMapPassSettingsData_));
	assert(shadowMapPassSettingsData_);
	shadowMapPassSettingsData_->shadowType = kShadowMapPassDirectional;
	std::fill(std::begin(shadowMapPassSettingsData_->padding), std::end(shadowMapPassSettingsData_->padding), 0.0f);
	shadowMapPassSettingsResource_->Unmap(0, nullptr);
	shadowMapPassSettingsData_ = nullptr;

	SetPointLights(nullptr, 0);
	SetSpotLights(nullptr, 0);
	SetAreaLights(nullptr, 0);

	editorDirectionalLight_ = *directionalLightData_;
	editorPointLightCount_ = 0;
	editorSpotLightCount_ = 0;
	editorAreaLightCount_ = 0;
	std::fill(editorPointLights_.begin(), editorPointLights_.end(), PointCommonLight{});
	std::fill(editorSpotLights_.begin(), editorSpotLights_.end(), SpotCommonLight{});
	std::fill(editorAreaLights_.begin(), editorAreaLights_.end(), AreaCommonLight{});
	D3D12_HEAP_PROPERTIES heapProps{};
	heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;

	D3D12_RESOURCE_DESC shadowDesc{};
	shadowDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	shadowDesc.Width = kShadowMapSize_;
	shadowDesc.Height = kShadowMapSize_;
	shadowDesc.DepthOrArraySize = 1;
	shadowDesc.MipLevels = 1;
	shadowDesc.Format = DXGI_FORMAT_R32_TYPELESS;
	shadowDesc.SampleDesc.Count = 1;
	shadowDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	shadowDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	D3D12_CLEAR_VALUE clearValue{};
	clearValue.Format = DXGI_FORMAT_D32_FLOAT;
	clearValue.DepthStencil.Depth = 1.0f;

	CreateShadowRenderTarget(dxCommon_, heapProps, shadowDesc, clearValue, {&directionalShadowMapResource_, &directionalShadowDsvHeap_, &directionalShadowMapSrvIndex_});
	CreateShadowRenderTarget(dxCommon_, heapProps, shadowDesc, clearValue, {&pointShadowMapResource_, &pointShadowDsvHeap_, &pointShadowMapSrvIndex_});
	CreateShadowRenderTarget(dxCommon_, heapProps, shadowDesc, clearValue, {&spotShadowMapResource_, &spotShadowDsvHeap_, &spotShadowMapSrvIndex_});
	CreateShadowRenderTarget(dxCommon_, heapProps, shadowDesc, clearValue, {&areaShadowMapResource_, &areaShadowDsvHeap_, &areaShadowMapSrvIndex_});

	shadowViewport_.TopLeftX = 0.0f;
	shadowViewport_.TopLeftY = 0.0f;
	shadowViewport_.Width = static_cast<float>(kShadowMapSize_);
	shadowViewport_.Height = static_cast<float>(kShadowMapSize_);
	shadowViewport_.MinDepth = 0.0f;
	shadowViewport_.MaxDepth = 1.0f;
	shadowScissorRect_.left = 0;
	shadowScissorRect_.top = 0;
	shadowScissorRect_.right = static_cast<LONG>(kShadowMapSize_);
	shadowScissorRect_.bottom = static_cast<LONG>(kShadowMapSize_);
}
void Object3dCommon::SetEnvironmentMapTexture(const std::string& filePath) {
	environmentMapPath_ = filePath;
	TextureManager::GetInstance()->LoadTextureName(environmentMapPath_);
	environmentMapSrvIndex_ = TextureManager::GetInstance()->GetTextureIndexByfilePath(environmentMapPath_);
}
void Object3dCommon::SetEnvironmentMapTextureResource(ID3D12Resource* resource, DXGI_FORMAT format) {
	if (!resource) {
		return;
	}
	environmentMapSrvIndex_ = TextureManager::GetInstance()->GetSrvManager()->Allocate();
	TextureManager::GetInstance()->GetSrvManager()->CreateSRVforTexture2D(environmentMapSrvIndex_, resource, format, 1);
}
void Object3dCommon::DrawSet(){
	if (useEditorLights_) {
		SetDirectionalLight(editorDirectionalLight_);
		SetPointLights(editorPointLights_.data(), editorPointLightCount_);
		SetSpotLights(editorSpotLights_.data(), editorSpotLightCount_);
		SetAreaLights(editorAreaLights_.data(), editorAreaLightCount_);
	}
	dxCommon_->GetCommandList()->SetGraphicsRootConstantBufferView(3, Object3dCommon::GetInstance()->GetDirectionalLightResource()->GetGPUVirtualAddress());
	dxCommon_->GetCommandList()->SetGraphicsRootConstantBufferView(5, Object3dCommon::GetInstance()->GetPointLightCountResource()->GetGPUVirtualAddress());
	dxCommon_->GetCommandList()->SetGraphicsRootConstantBufferView(6, Object3dCommon::GetInstance()->GetSpotLightCountResource()->GetGPUVirtualAddress());
	dxCommon_->GetCommandList()->SetGraphicsRootConstantBufferView(7, Object3dCommon::GetInstance()->GetAreaLightCountResource()->GetGPUVirtualAddress());
	dxCommon_->GetCommandList()->SetGraphicsRootConstantBufferView(16, Object3dCommon::GetInstance()->GetShadowMapPassSettingsResource()->GetGPUVirtualAddress());
	if (!isShadowMapPassActive_) {
		TextureManager::GetInstance()->GetSrvManager()->SetGraphicsRootDescriptorTable(12, directionalShadowMapSrvIndex_);
		TextureManager::GetInstance()->GetSrvManager()->SetGraphicsRootDescriptorTable(13, pointShadowMapSrvIndex_);
		TextureManager::GetInstance()->GetSrvManager()->SetGraphicsRootDescriptorTable(14, spotShadowMapSrvIndex_);
		TextureManager::GetInstance()->GetSrvManager()->SetGraphicsRootDescriptorTable(15, areaShadowMapSrvIndex_);

	}
}
void Object3dCommon::DrawCommon() {

	dxCommon_->GetCommandList()->SetGraphicsRootSignature(pso_->GetRootSignature().Get());
	dxCommon_->GetCommandList()->SetPipelineState(pso_->GetGraphicsPipelineState(blendMode_).Get());
	DrawSet();
	dxCommon_->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}
void Object3dCommon::DrawCommon(Camera* camera) {
	Camera* previousCamera = defaultCamera;
	if (camera) {
		defaultCamera = camera;
	}
	DrawCommon();
	defaultCamera = previousCamera;
}
void Object3dCommon::DrawCommonEmissive() {
	dxCommon_->GetCommandList()->SetGraphicsRootSignature(psoEmissive_->GetRootSignature().Get());
	dxCommon_->GetCommandList()->SetPipelineState(psoEmissive_->GetGraphicsPipelineState(blendMode_).Get());
	DrawSet();
	dxCommon_->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}
void Object3dCommon::DrawCommonNoCull() {

	dxCommon_->GetCommandList()->SetGraphicsRootSignature(psoNoCull_->GetRootSignature().Get());
	dxCommon_->GetCommandList()->SetPipelineState(psoNoCull_->GetGraphicsPipelineState(blendMode_).Get());
	DrawSet();
	dxCommon_->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}
void Object3dCommon::DrawCommonNoDepth() {
	dxCommon_->GetCommandList()->SetGraphicsRootSignature(psoNoDepth_->GetRootSignature().Get());
	dxCommon_->GetCommandList()->SetPipelineState(psoNoDepth_->GetGraphicsPipelineState(blendMode_).Get());
	DrawSet();
	dxCommon_->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}
void Object3dCommon::DrawCommonToon() {

	dxCommon_->GetCommandList()->SetGraphicsRootSignature(psoToon_->GetRootSignature().Get());
	dxCommon_->GetCommandList()->SetPipelineState(psoToon_->GetGraphicsPipelineState(blendMode_).Get());
	DrawSet();
	dxCommon_->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}
void Object3dCommon::DrawCommonNoCullDepth() {
	dxCommon_->GetCommandList()->SetGraphicsRootSignature(psoNoCullDepth_->GetRootSignature().Get());
	dxCommon_->GetCommandList()->SetPipelineState(psoNoCullDepth_->GetGraphicsPipelineState(blendMode_).Get());
	DrawSet();
	dxCommon_->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}
void Object3dCommon::DrawCommonWireframeNoDepth() {
	dxCommon_->GetCommandList()->SetGraphicsRootSignature(psoWireframeNoDepth_->GetRootSignature().Get());
	dxCommon_->GetCommandList()->SetPipelineState(psoWireframeNoDepth_->GetGraphicsPipelineState(blendMode_).Get());
	DrawSet();
	dxCommon_->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}
void Object3dCommon::DrawCommonLineNoDepth() {
	dxCommon_->GetCommandList()->SetGraphicsRootSignature(psoLineNoDepth_->GetRootSignature().Get());
	dxCommon_->GetCommandList()->SetPipelineState(psoLineNoDepth_->GetGraphicsPipelineState(blendMode_).Get());
	DrawSet();
	dxCommon_->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);
}
void Object3dCommon::DrawCommonEditorGrid() {
	dxCommon_->GetCommandList()->SetGraphicsRootSignature(psoEditorGrid_->GetRootSignature().Get());
	dxCommon_->GetCommandList()->SetPipelineState(psoEditorGrid_->GetGraphicsPipelineState(blendMode_).Get());
	DrawSet();
	dxCommon_->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}
void Object3dCommon::DrawCommonSkinning() {

	dxCommon_->GetCommandList()->SetGraphicsRootSignature(psoSkinning_->GetRootSignature().Get());
	dxCommon_->GetCommandList()->SetPipelineState(psoSkinning_->GetGraphicsPipelineState(blendMode_).Get());
	DrawSet();
	dxCommon_->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}
void Object3dCommon::DrawCommonSkinningToon() {

	dxCommon_->GetCommandList()->SetGraphicsRootSignature(psoSkinningToon_->GetRootSignature().Get());
	dxCommon_->GetCommandList()->SetPipelineState(psoSkinningToon_->GetGraphicsPipelineState(blendMode_).Get());
	DrawSet();
	dxCommon_->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}
void Object3dCommon::DrawCommonSkinningOutline() {
	dxCommon_->BeginOutlineRenderTarget();
	dxCommon_->GetCommandList()->SetGraphicsRootSignature(psoSkinningOutline_->GetRootSignature().Get());
	dxCommon_->GetCommandList()->SetPipelineState(psoSkinningOutline_->GetGraphicsPipelineState(blendMode_).Get());
	DrawSet();
	dxCommon_->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}
void Object3dCommon::DrawCommonSkinningToonOutline() {
	dxCommon_->BeginOutlineRenderTarget();
	dxCommon_->GetCommandList()->SetGraphicsRootSignature(psoSkinningToonOutline_->GetRootSignature().Get());
	dxCommon_->GetCommandList()->SetPipelineState(psoSkinningToonOutline_->GetGraphicsPipelineState(blendMode_).Get());
	DrawSet();
	dxCommon_->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}
void Object3dCommon::DrawCommonMirror() {

	dxCommon_->GetCommandList()->SetGraphicsRootSignature(psoMirror_->GetRootSignature().Get());
	dxCommon_->GetCommandList()->SetPipelineState(psoMirror_->GetGraphicsPipelineState(blendMode_).Get());
	DrawSet();
	dxCommon_->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}
void Object3dCommon::DrawCommonOutline() {
	dxCommon_->BeginOutlineRenderTarget();
	dxCommon_->GetCommandList()->SetGraphicsRootSignature(psoOutline_->GetRootSignature().Get());
	dxCommon_->GetCommandList()->SetPipelineState(psoOutline_->GetGraphicsPipelineState(blendMode_).Get());
	DrawSet();
	dxCommon_->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}
void Object3dCommon::EndOutlineDraw() { dxCommon_->EndOutlineRenderTarget(); }
void Object3dCommon::DrawCommonPortal() {

	dxCommon_->GetCommandList()->SetGraphicsRootSignature(psoPortal_->GetRootSignature().Get());
	dxCommon_->GetCommandList()->SetPipelineState(psoPortal_->GetGraphicsPipelineState(blendMode_).Get());
	DrawSet();
	dxCommon_->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}
void Object3dCommon::DrawCommonPortal(Camera* camera) {
	Camera* previousCamera = defaultCamera;
	if (camera) {
		defaultCamera = camera;
	}
	DrawCommonPortal();
	defaultCamera = previousCamera;
}
void Object3dCommon::DrawCommonShadow() {
	dxCommon_->GetCommandList()->SetGraphicsRootSignature(psoShadow_->GetRootSignature().Get());
	dxCommon_->GetCommandList()->SetPipelineState(psoShadow_->GetGraphicsPipelineState(blendMode_).Get());
	DrawSet();
	dxCommon_->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void Object3dCommon::BeginShadowMapPass() {
	ID3D12Resource* resource = directionalShadowEnabled_ ? directionalShadowMapResource_.Get()
	                                                     : (pointShadowEnabled_ ? pointShadowMapResource_.Get()
	                                                                            : (spotShadowEnabled_ ? spotShadowMapResource_.Get() : (areaShadowEnabled_ ? areaShadowMapResource_.Get() : nullptr)));
	ID3D12DescriptorHeap* dsvHeap =
	    directionalShadowEnabled_ ? directionalShadowDsvHeap_.Get()
	                              : (pointShadowEnabled_ ? pointShadowDsvHeap_.Get() : (spotShadowEnabled_ ? spotShadowDsvHeap_.Get() : (areaShadowEnabled_ ? areaShadowDsvHeap_.Get() : nullptr)));
	if (!resource || !dsvHeap) {
		return;
	}
	isShadowMapPassActive_ = true;
	TransitionShadowResource(dxCommon_, resource, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_DEPTH_WRITE);
	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = dsvHeap->GetCPUDescriptorHandleForHeapStart();
	dxCommon_->GetCommandList()->OMSetRenderTargets(0, nullptr, false, &dsvHandle);
	dxCommon_->GetCommandList()->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
	dxCommon_->GetCommandList()->RSSetViewports(1, &shadowViewport_);
	dxCommon_->GetCommandList()->RSSetScissorRects(1, &shadowScissorRect_);
}

void Object3dCommon::EndShadowMapPass() {
	ID3D12Resource* resource = directionalShadowEnabled_ ? directionalShadowMapResource_.Get()
	                                                     : (pointShadowEnabled_ ? pointShadowMapResource_.Get()
	                                                                            : (spotShadowEnabled_ ? spotShadowMapResource_.Get() : (areaShadowEnabled_ ? areaShadowMapResource_.Get() : nullptr)));
	if (!resource) {
		return;
	}
	TransitionShadowResource(dxCommon_, resource, D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	isShadowMapPassActive_ = false;
	TextureManager::GetInstance()->GetSrvManager()->PreDraw();
}

void Object3dCommon::SetDirectionalLight(DirectionalCommonLight& light) {
	*directionalLightData_ = light;
	directionalLightData_->direction = Function::Normalize(directionalLightData_->direction);
}

Matrix4x4 Object3dCommon::GetDirectionalLightViewProjectionMatrix() const {
	Vector3 lightDirection = Function::Normalize(directionalLightData_->direction);
	if (Function::Length(lightDirection) < 1.0e-5f) {
		lightDirection = {0.0f, -1.0f, 0.0f};
	}
	const Vector3 lightPosition = shadowLightPosition_ - lightDirection * 120.0f;
	const Vector3 up = (std::abs(lightDirection.y) > 0.99f) ? Vector3{0.0f, 0.0f, 1.0f} : Vector3{0.0f, 1.0f, 0.0f};
	const Vector3 right = Function::Normalize(Function::Cross(up, lightDirection));
	const Vector3 cameraUp = Function::Cross(lightDirection, right);

	Matrix4x4 lightView = Function::MakeIdentity4x4();
	lightView.m[0][0] = right.x;
	lightView.m[1][0] = right.y;
	lightView.m[2][0] = right.z;
	lightView.m[0][1] = cameraUp.x;
	lightView.m[1][1] = cameraUp.y;
	lightView.m[2][1] = cameraUp.z;
	lightView.m[0][2] = lightDirection.x;
	lightView.m[1][2] = lightDirection.y;
	lightView.m[2][2] = lightDirection.z;
	lightView.m[3][0] = -Function::Dot(lightPosition, right);
	lightView.m[3][1] = -Function::Dot(lightPosition, cameraUp);
	lightView.m[3][2] = -Function::Dot(lightPosition, lightDirection);

	Matrix4x4 lightProjection = Function::MakeOrthographicMatrix(-shadowOrthoHalfWidth_, shadowOrthoHalfHeight_, shadowOrthoHalfWidth_, -shadowOrthoHalfHeight_, shadowCameraNear_, shadowCameraFar_);
	return Function::Multiply(lightView, lightProjection);
}
Matrix4x4 Object3dCommon::GetPointLightViewProjectionMatrix() const {
	Vector3 lightPosition = shadowLightPosition_;
	Vector3 lightDirection = Function::Normalize(-lightPosition);
	float farPlane = shadowCameraFar_;
	float fov = Function::kPi * 0.6f;

	const PointCommonLight* selectedLight = nullptr;
	for (uint32_t i = 0; i < cachedPointLightCount_; ++i) {
		if (cachedPointLights_[i].shadowEnabled != 0) {
			selectedLight = &cachedPointLights_[i];
			break;
		}
	}
	if (!selectedLight && cachedPointLightCount_ > 0) {
		selectedLight = &cachedPointLights_[0];
	}
	if (selectedLight) {
		lightPosition = selectedLight->position;
		farPlane = std::max(shadowCameraNear_ + 0.1f, selectedLight->radius);
		if (Function::Length(selectedLight->position) > 1.0e-4f) {
			lightDirection = Function::Normalize(-selectedLight->position);
		} else {
			lightDirection = {0.0f, -1.0f, 0.0f};
		}
	}
	return MakeLightViewProjection(lightPosition, lightDirection, shadowCameraNear_, farPlane, fov);
}

Matrix4x4 Object3dCommon::GetSpotLightViewProjectionMatrix() const {
	Vector3 lightPosition = shadowLightPosition_;
	Vector3 lightDirection = {0.0f, -1.0f, 0.0f};
	float fov = 0.9f;
	float farPlane = shadowCameraFar_;

	const SpotCommonLight* selectedLight = nullptr;
	for (uint32_t i = 0; i < cachedSpotLightCount_; ++i) {
		if (cachedSpotLights_[i].shadowEnabled != 0) {
			selectedLight = &cachedSpotLights_[i];
			break;
		}
	}
	if (!selectedLight && cachedSpotLightCount_ > 0) {
		selectedLight = &cachedSpotLights_[0];
	}
	if (selectedLight) {
		lightPosition = selectedLight->position;
		lightDirection = Function::Normalize(selectedLight->direction);
		const float outerAngle = std::acos(std::clamp(selectedLight->cosAngle, -1.0f, 1.0f));
		fov = outerAngle * 2.0f + 0.1f;
		farPlane = std::max(shadowCameraNear_ + 0.1f, selectedLight->distance);
	}
	return MakeLightViewProjection(lightPosition, lightDirection, shadowCameraNear_, farPlane, fov);
}

Matrix4x4 Object3dCommon::GetAreaLightViewProjectionMatrix() const {
	Vector3 lightPosition = shadowLightPosition_;
	Vector3 lightDirection = {0.0f, -1.0f, 0.0f};
	float fov = Function::kPi * 0.5f;
	float farPlane = shadowCameraFar_;

	const AreaCommonLight* selectedLight = nullptr;
	for (uint32_t i = 0; i < cachedAreaLightCount_; ++i) {
		if (cachedAreaLights_[i].shadowEnabled != 0) {
			selectedLight = &cachedAreaLights_[i];
			break;
		}
	}
	if (!selectedLight && cachedAreaLightCount_ > 0) {
		selectedLight = &cachedAreaLights_[0];
	}
	if (selectedLight) {
		lightPosition = selectedLight->position;
		lightDirection = -Function::Normalize(selectedLight->normal);
		const float coverageRadius = std::max(selectedLight->radius, std::max(selectedLight->width, selectedLight->height));
		fov = std::atan2(std::max(0.5f, coverageRadius), std::max(0.1f, selectedLight->radius)) * 2.0f + 0.1f;
		farPlane = std::max({shadowCameraNear_ + 0.1f, selectedLight->radius, selectedLight->width, selectedLight->height});
	}
	return MakeLightViewProjection(lightPosition, lightDirection, shadowCameraNear_, farPlane, fov);
}

void Object3dCommon::SetBlendMode(BlendMode blendMode) {
	blendMode_ = blendMode;
	dxCommon_->GetCommandList()->SetPipelineState(pso_->GetGraphicsPipelineState(blendMode_).Get());
}
void Object3dCommon::SetPointLights(const PointCommonLight* pointLights, uint32_t count) {
	uint32_t clampedCount = std::min(count, static_cast<uint32_t>(kMaxPointLights));
	cachedPointLightCount_ = clampedCount;
	std::fill(cachedPointLights_.begin(), cachedPointLights_.end(), PointCommonLight{});
	if (pointLights && clampedCount > 0) {
		std::copy_n(pointLights, clampedCount, cachedPointLights_.begin());
	}
	pointLightResource_->Map(0, nullptr, reinterpret_cast<void**>(&pointlightData_));
	std::memset(pointlightData_, 0, sizeof(PointCommonLight) * kMaxPointLights);
	if (pointLights && clampedCount > 0) {
		std::memcpy(pointlightData_, pointLights, sizeof(PointCommonLight) * clampedCount);
	}
	pointLightResource_->Unmap(0, nullptr);

	pointLightCountResource_->Map(0, nullptr, reinterpret_cast<void**>(&pointLightCountData_));
	pointLightCountData_->count = clampedCount;
	pointLightCountResource_->Unmap(0, nullptr);
}
void Object3dCommon::SetSpotLights(const SpotCommonLight* spotLights, uint32_t count) {
	uint32_t clampedCount = std::min(count, static_cast<uint32_t>(kMaxSpotLights));
	cachedSpotLightCount_ = clampedCount;
	std::fill(cachedSpotLights_.begin(), cachedSpotLights_.end(), SpotCommonLight{});
	if (spotLights && clampedCount > 0) {
		std::copy_n(spotLights, clampedCount, cachedSpotLights_.begin());
	}
	spotLightResource_->Map(0, nullptr, reinterpret_cast<void**>(&spotLightData_));
	std::memset(spotLightData_, 0, sizeof(SpotCommonLight) * kMaxSpotLights);
	if (spotLights && clampedCount > 0) {
		std::memcpy(spotLightData_, spotLights, sizeof(SpotCommonLight) * clampedCount);
	}
	spotLightResource_->Unmap(0, nullptr);

	spotLightCountResource_->Map(0, nullptr, reinterpret_cast<void**>(&spotLightCountData_));
	spotLightCountData_->count = clampedCount;
	spotLightCountResource_->Unmap(0, nullptr);
}
void Object3dCommon::SetAreaLights(const AreaCommonLight* areaLights, uint32_t count) {
	uint32_t clampedCount = std::min(count, static_cast<uint32_t>(kMaxAreaLights));
	cachedAreaLightCount_ = clampedCount;
	std::fill(cachedAreaLights_.begin(), cachedAreaLights_.end(), AreaCommonLight{});
	if (areaLights && clampedCount > 0) {
		std::copy_n(areaLights, clampedCount, cachedAreaLights_.begin());
	}
	areaLightResource_->Map(0, nullptr, reinterpret_cast<void**>(&areaLightData_));
	std::memset(areaLightData_, 0, sizeof(AreaCommonLight) * kMaxAreaLights);
	if (areaLights && clampedCount > 0) {
		std::memcpy(areaLightData_, areaLights, sizeof(AreaCommonLight) * clampedCount);
	}
	areaLightResource_->Unmap(0, nullptr);

	areaLightCountResource_->Map(0, nullptr, reinterpret_cast<void**>(&areaLightCountData_));
	areaLightCountData_->count = clampedCount;
	areaLightCountResource_->Unmap(0, nullptr);
}
void Object3dCommon::SetEditorLights(
    const DirectionalCommonLight& directionalLight, const PointCommonLight* pointLights, uint32_t pointCount, const SpotCommonLight* spotLights, uint32_t spotCount, const AreaCommonLight* areaLights, uint32_t areaCount) {
	editorDirectionalLight_ = directionalLight;
	editorDirectionalLight_.direction = Function::Normalize(editorDirectionalLight_.direction);
	if (Function::Length(editorDirectionalLight_.direction) < 1.0e-5f) {
		editorDirectionalLight_.direction = {0.0f, -1.0f, 0.0f};
	}

	editorPointLightCount_ = std::min(pointCount, static_cast<uint32_t>(kMaxPointLights));
	std::fill(editorPointLights_.begin(), editorPointLights_.end(), PointCommonLight{});
	if (pointLights && editorPointLightCount_ > 0) {
		std::copy_n(pointLights, editorPointLightCount_, editorPointLights_.begin());
	}

	editorSpotLightCount_ = std::min(spotCount, static_cast<uint32_t>(kMaxSpotLights));
	std::fill(editorSpotLights_.begin(), editorSpotLights_.end(), SpotCommonLight{});
	if (spotLights && editorSpotLightCount_ > 0) {
		std::copy_n(spotLights, editorSpotLightCount_, editorSpotLights_.begin());
	}

	editorAreaLightCount_ = std::min(areaCount, static_cast<uint32_t>(kMaxAreaLights));
	std::fill(editorAreaLights_.begin(), editorAreaLights_.end(), AreaCommonLight{});
	if (areaLights && editorAreaLightCount_ > 0) {
		std::copy_n(areaLights, editorAreaLightCount_, editorAreaLights_.begin());
	}
}
void Object3dCommon::SetVignetteStrength(float strength) { dxCommon_->SetVignetteStrength(strength); }

void Object3dCommon::SetRandomNoiseEnabled(bool enabled) { dxCommon_->SetRandomNoiseEnabled(enabled); }

void Object3dCommon::SetRandomNoiseScale(float scale) { dxCommon_->SetRandomNoiseScale(scale); }

void Object3dCommon::SetRandomNoiseBlendMode(int blendMode) { dxCommon_->SetRandomNoiseBlendMode(blendMode); }

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