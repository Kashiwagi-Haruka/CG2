#pragma once
#include <string>
#include <DirectXTex.h>
#include <d3d12.h>
#include <format>
#include <wrl.h>
class Texture{

	public:


	int TexInitialize(ID3D12Device* device_, ID3D12DescriptorHeap* srvDescriptorHeap_, const std::string& fileName);
	int ModelTexInitialize(ID3D12Device* device, ID3D12DescriptorHeap* srvDescriptorHeap, const std::string& filaname);

	DirectX::ScratchImage LoadTextureName(const std::string& filePath);
	Microsoft::WRL::ComPtr <ID3D12Resource> CreateTextureResource(ID3D12Device* device_, const DirectX::TexMetadata& metadata);
	void UploadTextureData(ID3D12Resource* texture, const DirectX::ScratchImage& mipImages);
	void Finalize();
	D3D12_GPU_DESCRIPTOR_HANDLE GetTexGpuHandle();
	D3D12_GPU_DESCRIPTOR_HANDLE GetModelGpuHandle();
	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandle(ID3D12DescriptorHeap* descriptorHeap, uint32_t descriptorSize, uint32_t index);
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandle(ID3D12DescriptorHeap* descriptorHeap, uint32_t descriptorSize, uint32_t index);

	void SetFilePath(std::string filePath) { filePath_ = filePath; };
	std::string& GetFilePath() { return filePath_; };


	int GetTextureToTal() { return TextureTotal; };
	int GetmodelTexTotal() { return modelTexTotal; };

private:
	uint32_t descriptorSizeSRV = 0;
	uint32_t descriptorSizeRTV = 0;
	uint32_t descriptorSizeDSV = 0;

	
	std::string filePath_;
	
	Microsoft::WRL::ComPtr < ID3D12Resource> textureResource_;
	D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandleGPU_{}; // SRV用GPUハンドル
	D3D12_GPU_DESCRIPTOR_HANDLE modelTexSrvHandleGPU_{};
	

	int TextureTotal = 0;
	int modelTexTotal = 0;
};
