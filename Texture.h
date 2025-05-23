#pragma once
#include <string>
#include <DirectXTex.h>
#include "ConvertString.h"
#include <d3d12.h>
#include <format>
class Texture{

	public:

	void Initialize(ID3D12Device* device_, ID3D12DescriptorHeap* srvDescriptorHeap_, const std::string& fileName);

	DirectX::ScratchImage LoadTexture(const std::string& filePath);
	ID3D12Resource* CreateTextureResource(ID3D12Device* device_, const DirectX::TexMetadata& metadata);
	void UploadTextureData(ID3D12Resource* texture, const DirectX::ScratchImage& mipImages);

	D3D12_GPU_DESCRIPTOR_HANDLE GetGpuHandle();

	void Release();

	

private:
	ID3D12Resource* textureResource_ = nullptr;

	ConvertString Cstr_;
	
	D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandleGPU_{}; // SRV用GPUハンドル

};
