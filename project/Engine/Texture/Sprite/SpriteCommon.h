#pragma once
#include "BlendMode/BlendModeManager.h"
#include "PSO/SpriteCreatePSO.h"
#include <Windows.h>
#include <memory>
#include <wrl.h>
class DirectXCommon;

class SpriteCommon {

private:
	static std::unique_ptr<SpriteCommon> instance_;

	DirectXCommon* dxCommon_;

	BlendMode blendMode_ = BlendMode::kBlendModeAlpha;
	BlendModeManager blendModeManager_;

	std::unique_ptr<SpriteCreatePSO> pso_;
	std::unique_ptr<SpriteCreatePSO> psoFont_;
	bool isSpriteVisible_ = true;

	HRESULT hr_;

public:
	static SpriteCommon* GetInstance();
	void Initialize(DirectXCommon* dxCommon);
	void Finalize();
	void DrawCommon();
	void DrawCommonFont();
	void SetBlendMode(const BlendMode& blendMode = BlendMode::kBlendModeAlpha) { blendMode_ = blendMode; }
	void SetSpriteVisible(bool isVisible) { isSpriteVisible_ = isVisible; }
	bool IsSpriteVisible() const { return isSpriteVisible_; }
	Microsoft::WRL::ComPtr<ID3D12Resource> CreateBufferResource(size_t sizeInBytes);

	DirectXCommon* GetDxCommon() const { return dxCommon_; };
};