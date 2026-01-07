#pragma once
#include "Windows.h"
#include"wrl.h"
#include <d3d12.h>
#include "BlendMode/BlendModeManager.h"
class DirectXCommon;
class CreatePSO {


	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> graphicsPipelineState_[6];

	Microsoft::WRL::ComPtr<ID3DBlob> signatureBlob_;
	Microsoft::WRL::ComPtr<ID3DBlob> errorBlob_;

	DirectXCommon* dxCommon_ = nullptr;

	HRESULT hr_;

	BlendMode blendMode_;
	BlendModeManager blendModeManager_;

	void CreateRootSignature();
	void CreateGraphicsPipeline(D3D12_CULL_MODE cullMode);

	public:
	
	CreatePSO(DirectXCommon* dxCommom);
	    void Create(D3D12_CULL_MODE cullMode);
	Microsoft::WRL::ComPtr<ID3D12RootSignature> GetRootSignature() { return rootSignature_; };
	Microsoft::WRL::ComPtr<ID3D12PipelineState> GetGraphicsPipelineState(BlendMode blendMode) { return graphicsPipelineState_[blendMode]; };

};
