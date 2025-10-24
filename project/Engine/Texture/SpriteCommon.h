#pragma once
#include <Windows.h>
#include <wrl.h>
#include "BlendModeManeger.h"
class DirectXCommon;

class BlendmodeManeger;

class SpriteCommon {

	private:

	
	DirectXCommon* dxCommon_;
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> graphicsPipelineState_[6];

	Microsoft::WRL::ComPtr<ID3DBlob> signatureBlob_;
	Microsoft::WRL::ComPtr<ID3DBlob> errorBlob_;

	BlendMode blendMode_;
	BlendModeManeger blendModeManeger_;

	HRESULT hr_;

	public:
	void Initialize(DirectXCommon* dxCommon);
	void DrawCommon();

	void CreateRootSignatures();

	void CreateGraphicsPipeline();

	Microsoft::WRL::ComPtr<ID3D12Resource> CreateBufferResource(size_t sizeInBytes);

	DirectXCommon* GetDxCommon() const { return dxCommon_; };

};
