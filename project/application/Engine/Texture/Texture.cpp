#include "Texture.h"


void Texture::Initialize(ID3D12Device* device_, ID3D12DescriptorHeap* srvDescriptorHeap_, const std::string& fileName,uint32_t index) {
	descriptorSizeSRV = device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	descriptorSizeRTV = device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	descriptorSizeDSV = device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	// Textureを読んで転送する
	DirectX::ScratchImage mipImages = LoadTexture(fileName); // ←これが正しい

	const DirectX::TexMetadata& metadata = mipImages.GetMetadata();
	textureResource_ = CreateTextureResource(device_, metadata);
	UploadTextureData(textureResource_.Get(), mipImages);

	// --- SRV作成用にmeta情報を使ってView記述 ---
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = metadata.format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = UINT(metadata.mipLevels);

	// --- ヒープの位置を取得 ---
	D3D12_CPU_DESCRIPTOR_HANDLE textureSrvHandleCPU = GetCPUDescriptorHandle(srvDescriptorHeap_,descriptorSizeSRV,index)
		
		= srvDescriptorHeap_->GetCPUDescriptorHandleForHeapStart();
	textureSrvHandleGPU_ = srvDescriptorHeap_->GetGPUDescriptorHandleForHeapStart();

	// --- ImGuiが0番を使ってる場合、1つインクリメントして避ける ---
	UINT descriptorSize = device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	textureSrvHandleCPU.ptr += descriptorSize*index;
	textureSrvHandleGPU_.ptr += descriptorSize*index;

	// --- SRV作成 ---
	device_->CreateShaderResourceView(textureResource_.Get(), &srvDesc, textureSrvHandleCPU);
	assert(metadata.format != DXGI_FORMAT_UNKNOWN);

	OutputDebugStringA(std::format("GPU Handle: 0x{:X}\n", textureSrvHandleGPU_.ptr).c_str());
}

DirectX::ScratchImage Texture::LoadTexture(const std::string& filePath) {
		// テクスチャファイルを読んでプログラムで使えるようにする
		DirectX::ScratchImage image{};
		std::wstring filePathW = Cstr.ConvertString_(filePath);
		HRESULT hr_ = DirectX::LoadFromWICFile(filePathW.c_str(), DirectX::WIC_FLAGS_FORCE_SRGB, nullptr, image);
		assert(SUCCEEDED(hr_));

		// ミップマップの作成
		DirectX::ScratchImage mipImages{};
		hr_ = DirectX::GenerateMipMaps(image.GetImages(), image.GetImageCount(), image.GetMetadata(), DirectX::TEX_FILTER_SRGB, 0, mipImages);
		assert(SUCCEEDED(hr_));

		// ミップマップ付きテクスチャを返す
		return mipImages;
}

Microsoft::WRL::ComPtr < ID3D12Resource> Texture::CreateTextureResource(ID3D12Device* device_, const DirectX::TexMetadata& metadata) {
	
	// metadataを基にResourceの設定
	D3D12_RESOURCE_DESC resourceDesc{};
	resourceDesc.Width = UINT(metadata.width);                             // Textureの幅
	resourceDesc.Height = UINT(metadata.height);                           // Textureの高さ
	resourceDesc.MipLevels = UINT16(metadata.mipLevels);                   // mipmapの数
	resourceDesc.DepthOrArraySize = UINT16(metadata.arraySize);            // 奥行き or 配列Textureの配列数
	resourceDesc.Format = metadata.format;                                 // TextureのFormat
	resourceDesc.SampleDesc.Count = 1;                                     // サンプリングカウント。1固定。
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION(metadata.dimension); // Textureの次元数。普段使っているのは2次元

	// 利用するHeapの設定。非常に特殊な運用。02_04exで一般的なケース版がある
	D3D12_HEAP_PROPERTIES heapProperties{};
	heapProperties.Type = D3D12_HEAP_TYPE_CUSTOM;                        // 細かい設定を行う
	heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_BACK; // WriteBackポリシーでCPUアクセス可能
	heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_L0;          // プロセッサの近くに配置

	// Resourceの生成
	textureResource_ = nullptr;
	HRESULT hr_ = device_->CreateCommittedResource(
	    &heapProperties,                   // Heapの設定
	    D3D12_HEAP_FLAG_NONE,              // Heapの特殊な設定。特になし。
	    &resourceDesc,                     // Resourceの設定
	    D3D12_RESOURCE_STATE_GENERIC_READ, // 初回のResourceState。Textureは基本読むだけ
	    nullptr,                           // Clear最適値。使わないのでnullptr
	    IID_PPV_ARGS(&textureResource_));          // 作成するResourceポインタへのポインタ
	assert(SUCCEEDED(hr_));
	return textureResource_;
}

D3D12_GPU_DESCRIPTOR_HANDLE Texture::GetGpuHandle() { 
	
	return textureSrvHandleGPU_; }

void Texture::UploadTextureData(ID3D12Resource* texture, const DirectX::ScratchImage& mipImages) {
	// Meta情報を取得
	const DirectX::TexMetadata& metadata = mipImages.GetMetadata();

	// 全MipMapについて
	for (size_t mipLevel = 0; mipLevel < metadata.mipLevels; ++mipLevel) {
		// MipMapLevelを指定して各Imageを取得
		const DirectX::Image* img = mipImages.GetImage(mipLevel, 0, 0);

		// Textureに転送
		HRESULT hr_ = texture->WriteToSubresource(
		    UINT(mipLevel),      // 全領域へコピー
		    nullptr,             // 元データアドレス
		    img->pixels,         // 1ラインサイズ
		    UINT(img->rowPitch), // 1枚サイズ
		    UINT(img->slicePitch));
		assert(SUCCEEDED(hr_));
	}
}
void Texture::Finalize() {
	if (textureResource_) {
		textureResource_.Reset();
		
	}
}
D3D12_CPU_DESCRIPTOR_HANDLE Texture::GetCPUDescriptorHandle(ID3D12DescriptorHeap* descriptorHeap, uint32_t descriptorSize, uint32_t index) {
	D3D12_CPU_DESCRIPTOR_HANDLE handleCPU = descriptorHeap->GetCPUDescriptorHandleForHeapStart();
	handleCPU.ptr += (descriptorSize * index);
	return handleCPU;
}
D3D12_GPU_DESCRIPTOR_HANDLE Texture::GetGPUDescriptorHandle(ID3D12DescriptorHeap* descriptorHeap, uint32_t descriptorSize, uint32_t index) {
	D3D12_GPU_DESCRIPTOR_HANDLE handleGPU = descriptorHeap->GetGPUDescriptorHandleForHeapStart();
	handleGPU.ptr += (descriptorSize * index);
	return handleGPU;
}