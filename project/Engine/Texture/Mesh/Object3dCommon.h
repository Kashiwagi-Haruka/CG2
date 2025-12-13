#pragma once
#include <Windows.h>
#include <wrl.h>
#include "BlendModeManeger.h"
#include "Light/DirectionalLight.h"
class Camera;
class DirectXCommon;

class Object3dCommon {

	private:

	Camera* defaultCamera = nullptr;

	DirectXCommon* dxCommon_=nullptr;

	HRESULT hr_;

	BlendMode blendMode_ = BlendMode::kBlendModeAlpha;
	BlendModeManeger blendModeManeger_;


	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> graphicsPipelineState_[6];

	Microsoft::WRL::ComPtr<ID3DBlob> signatureBlob_;
	Microsoft::WRL::ComPtr<ID3DBlob> errorBlob_;

	// Directional Light（共通）
	Microsoft::WRL::ComPtr<ID3D12Resource> directionalLightResource_ = nullptr;
	DirectionalLight* directionalLightData_ = nullptr;

	

	private:

	void CreateRootsignature();

	void CreateGraphicsPipeline();

	

	public:
	void Initialize(DirectXCommon* dxCommon);
	void SetDefaultCamera(Camera* camera) { this->defaultCamera = camera; }
	Camera* GetDefaultCamera() const { return defaultCamera; };
	void DrawCommon();
	Microsoft::WRL::ComPtr<ID3D12Resource> CreateBufferResource(size_t sizeInBytes);
	DirectXCommon* GetDxCommon() const { return dxCommon_; };
	void SetBlendMode(BlendMode blendmode);
	BlendMode GetBlendMode() const { return blendMode_;}
	ID3D12Resource* GetDirectionalLightResource() const { return directionalLightResource_.Get(); }
	void SetDirectionalLight(DirectionalLight& light);
};
