#pragma once
#include <Windows.h>
#include <wrl.h>
#include <d3d12.h>
class DirectXCommon;

class ModelCommon {

	DirectXCommon* dxCommon_;

public:

	void Initialize(DirectXCommon* dxCommon);
	DirectXCommon* GetDxCommon() const { return dxCommon_; };
	Microsoft::WRL::ComPtr<ID3D12Resource> CreateBufferResource(size_t sizeInBytes);




};
