#pragma once
#include <Windows.h>
#include <wrl.h>
#include <DirectXTex.h>
#include <d3d12.h>
#include <string>
#include <cstdint>

class DirectXCommon;

class TextureManager {

	private:

		struct TextureData {
		
			std::string filePath;
		    DirectX::TexMetadata metadata;
		    Microsoft::WRL::ComPtr<ID3D12Resource> resource;
		    D3D12_CPU_DESCRIPTOR_HANDLE srvHandleCPU;
		    D3D12_GPU_DESCRIPTOR_HANDLE srvHandleGPU;

		};

		std::vector<TextureData> textureDatas;


		static TextureManager* instance;
	    static uint32_t kSRVIndexTop;
	    TextureManager() = default;
	    ~TextureManager() = default;
	    TextureManager(TextureManager&) = delete;
	    TextureManager& operator=(TextureManager&) = delete;

	    Microsoft::WRL::ComPtr<ID3D12Resource> CreateTextureResource(DirectX::TexMetadata& metadata);
	    void UploadTextureData(ID3D12Resource* texture, const DirectX::ScratchImage& mipImages);

		DirectXCommon* dxCommon_ = nullptr;

	public:

		//シングルトンインスタンスの取得
	    static TextureManager* GetInstance();

		//初期化
	    void Initialize(DirectXCommon* dxCommon);
	    void LoadTextureName(const std::string& filePath);
		
	    uint32_t GetTextureIndexByfilePath(const std::string& filePath);
	    D3D12_GPU_DESCRIPTOR_HANDLE GetSrvHandleGPU(uint32_t textureIndex);
	    DirectX::TexMetadata& GetMetaData(uint32_t textureIndex);
		//終了
	    void Finalize();

};
