#pragma once
#include <Windows.h>
#include <wrl.h>
#include "BlendMode/BlendModeManager.h"
#include <memory>
class DirectXCommon;

class SpriteCommon {

	private:

	static std::unique_ptr<SpriteCommon> instance_;
	
	DirectXCommon* dxCommon_;
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> graphicsPipelineState_[6];

	Microsoft::WRL::ComPtr<ID3DBlob> signatureBlob_;
	Microsoft::WRL::ComPtr<ID3DBlob> errorBlob_;

	BlendMode blendMode_ = BlendMode::kBlendModeAlpha;
	BlendModeManager blendModeManager_;

	HRESULT hr_;

	public:
	static SpriteCommon* GetInstance();
	void Initialize(DirectXCommon* dxCommon);
	void Finalize();
	void DrawCommon();

	void CreateRootSignatures();

	void CreateGraphicsPipeline();

	Microsoft::WRL::ComPtr<ID3D12Resource> CreateBufferResource(size_t sizeInBytes);

	DirectXCommon* GetDxCommon() const { return dxCommon_; };

};
