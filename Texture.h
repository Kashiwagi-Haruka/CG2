#pragma once
#include <string>
#include <DirectXTex.h>
#include "ConvertString.h"
#include <d3d12.h>
#include <format>
class Texture{

	public:

	void Initialize(ID3D12Device* device, ID3D12DescriptorHeap* srvDescriptorHeap, const std::string& fileName);

	DirectX::ScratchImage LoadTexture(const std::string& filePath);
	ID3D12Resource* CreateTextureResource(ID3D12Device* device, const DirectX::TexMetadata& metadata);
	void UploadTextureData(ID3D12Resource* texture, const DirectX::ScratchImage& mipImages);

	D3D12_GPU_DESCRIPTOR_HANDLE GetGpuHandle();
	// Texture.h に追加
public:
	void Release();

	

private:
	// Texture.h に追加
private:
	ID3D12Resource* textureResource_ = nullptr;

	ConvertString Cstr;
	
	D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandleGPU_{}; // SRV用GPUハンドル

};
