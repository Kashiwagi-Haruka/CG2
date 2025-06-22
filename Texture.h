#pragma once
#include <string>
#include <DirectXTex.h>
#include "ConvertString.h"
#include <d3d12.h>
#include <format>
class Texture{

	public:

	void Initialize(ID3D12Device* device_, ID3D12DescriptorHeap* srvDescriptorHeap_, const std::string& fileName,uint32_t index);

	DirectX::ScratchImage LoadTexture(const std::string& filePath);
	ID3D12Resource* CreateTextureResource(ID3D12Device* device_, const DirectX::TexMetadata& metadata);
	void UploadTextureData(ID3D12Resource* texture, const DirectX::ScratchImage& mipImages);
	void Finalize();
	D3D12_GPU_DESCRIPTOR_HANDLE GetGpuHandle();
	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandle(ID3D12DescriptorHeap* descriptorHeap, uint32_t descriptorSize, uint32_t index);
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandle(ID3D12DescriptorHeap* descriptorHeap, uint32_t descriptorSize, uint32_t index);

	void SetFilePath(std::string filePath) { filePath_ = filePath; };
	std::string& GetFilePath() { return filePath_; };


private:
	uint32_t descriptorSizeSRV = 0;
	uint32_t descriptorSizeRTV = 0;
	uint32_t descriptorSizeDSV = 0;

	ConvertString Cstr;
	std::string filePath_;
	
	ID3D12Resource* textureResource_;
	D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandleGPU_{}; // SRV用GPUハンドル
	
};
