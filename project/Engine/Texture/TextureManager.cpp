#include "TextureManager.h"
#include "DirectXCommon.h"
#include "SrvManager/SrvManager.h"
#include "StringUtility.h"
std::unique_ptr<TextureManager> TextureManager::instance = nullptr;
uint32_t TextureManager::kSRVIndexTop = 1;

// TextureManager のシングルトンインスタンスを返す
TextureManager* TextureManager::GetInstance() {

	if (instance == nullptr) {
		instance = std::make_unique<TextureManager>();
	}
	return instance.get();
}

// 利用する DirectX 共通機能と SRV マネージャーを設定する
void TextureManager::Initialize(DirectXCommon* dxCommon) {
	dxCommon_ = dxCommon;
	srvManager_ = std::make_unique<SrvManager>();
	srvManager_->Initialize(dxCommon_);
	textureDatas.reserve(srvManager_->kMaxSRVCount_);
}

// インスタンスを破棄して終了処理を行う
void TextureManager::Finalize() {
	if (srvManager_) {
		srvManager_->Finalize();
		srvManager_.reset();
	}
	textureDatas.clear();
	dxCommon_ = nullptr;
	instance.reset();
}

// ファイルパス指定でテクスチャを読み込み、SRV を作成する
void TextureManager::LoadTextureName(const std::string& filePath) {

	if (textureDatas.contains(filePath)) {
		return;
	}
	assert(srvManager_->CanAllocate());

	// テクスチャファイルを読んでプログラムで使えるようにする
	DirectX::ScratchImage image{};
	std::wstring filePathW = StringUtility::ConvertString_(filePath);
	HRESULT hr_ = S_FALSE;
	const bool isDDS = filePathW.size() >= 4 && _wcsicmp(filePathW.c_str() + (filePathW.size() - 4), L".dds") == 0;
	if (isDDS) {
		hr_ = DirectX::LoadFromDDSFile(filePathW.c_str(), DirectX::DDS_FLAGS_NONE, nullptr, image);
	} else {
		hr_ = DirectX::LoadFromWICFile(filePathW.c_str(), DirectX::WIC_FLAGS_FORCE_SRGB, nullptr, image);
	}
	assert(SUCCEEDED(hr_));

	// ミップマップの作成
	DirectX::ScratchImage mipImages{};
	if (DirectX::IsCompressed(image.GetMetadata().format)) {
		mipImages = std::move(image);
	} else {
		hr_ = DirectX::GenerateMipMaps(image.GetImages(), image.GetImageCount(), image.GetMetadata(), DirectX::TEX_FILTER_SRGB, 0, mipImages);
		assert(SUCCEEDED(hr_));
	}

	TextureData& textureData = textureDatas[filePath];
	textureData.filePath = filePath;
	textureData.metadata = mipImages.GetMetadata();
	textureData.resource = CreateTextureResource(textureData.metadata);
	if (isDDS && textureData.metadata.dimension == DirectX::TEX_DIMENSION_TEXTURE2D && textureData.metadata.arraySize >= 6 && (textureData.metadata.arraySize % 6) == 0 &&
	    textureData.metadata.depth == 1) {
		textureData.metadata.miscFlags |= DirectX::TEX_MISC_TEXTURECUBE;
	}
	// ★ ここを UploadTextureData に統一
	UploadTextureData(textureData.resource.Get(), mipImages);

	// SRVハンドル設定
	textureData.srvIndex = srvManager_->Allocate();
	textureData.srvHandleCPU = srvManager_->GetCPUDescriptorHandle(textureData.srvIndex);
	textureData.srvHandleGPU = srvManager_->GetGPUDescriptorHandle(textureData.srvIndex);

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = textureData.metadata.format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	if (textureData.metadata.IsCubemap()) {
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
		srvDesc.TextureCube.MostDetailedMip = 0;
		srvDesc.TextureCube.MipLevels = UINT(textureData.metadata.mipLevels);
		srvDesc.TextureCube.ResourceMinLODClamp = 0.0f;
	} else {
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = UINT(textureData.metadata.mipLevels);
	}

	dxCommon_->GetDevice()->CreateShaderResourceView(textureData.resource.Get(), &srvDesc, textureData.srvHandleCPU);

	// デバッグログ出力
	std::string log = "Texture Loaded: " + filePath + " | SRV Index: " + std::to_string(textureData.srvIndex) + " | GPU Handle: " + std::to_string(textureData.srvHandleGPU.ptr) + "\n";
	OutputDebugStringA(log.c_str());
}

// メモリ上の画像データを読み込み、SRV を作成する
void TextureManager::LoadTextureFromMemory(const std::string& key, const uint8_t* data, size_t size) {
	if (textureDatas.contains(key)) {
		return;
	}
	assert(srvManager_->CanAllocate());

	DirectX::ScratchImage image{};
	HRESULT hr_ = DirectX::LoadFromWICMemory(data, size, DirectX::WIC_FLAGS_FORCE_SRGB, nullptr, image);
	assert(SUCCEEDED(hr_));

	DirectX::ScratchImage mipImages{};
	hr_ = DirectX::GenerateMipMaps(image.GetImages(), image.GetImageCount(), image.GetMetadata(), DirectX::TEX_FILTER_SRGB, 0, mipImages);
	assert(SUCCEEDED(hr_));

	TextureData& textureData = textureDatas[key];
	textureData.filePath = key;
	textureData.metadata = mipImages.GetMetadata();
	textureData.resource = CreateTextureResource(textureData.metadata);

	UploadTextureData(textureData.resource.Get(), mipImages);

	textureData.srvIndex = srvManager_->Allocate();
	textureData.srvHandleCPU = srvManager_->GetCPUDescriptorHandle(textureData.srvIndex);
	textureData.srvHandleGPU = srvManager_->GetGPUDescriptorHandle(textureData.srvIndex);

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = textureData.metadata.format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = UINT(textureData.metadata.mipLevels);

	dxCommon_->GetDevice()->CreateShaderResourceView(textureData.resource.Get(), &srvDesc, textureData.srvHandleCPU);

	std::string log = "Embedded Texture Loaded: " + key + " | SRV Index: " + std::to_string(textureData.srvIndex) + " | GPU Handle: " + std::to_string(textureData.srvHandleGPU.ptr) + "\n";
	OutputDebugStringA(log.c_str());
}

// RGBA8 の生配列からテクスチャを生成して SRV を作成する
void TextureManager::LoadTextureFromRGBA8(const std::string& key, uint32_t width, uint32_t height, const uint8_t* data) {
	if (textureDatas.contains(key)) {
		return;
	}
	assert(srvManager_->CanAllocate());

	DirectX::ScratchImage image{};
	HRESULT hr_ = image.Initialize2D(DXGI_FORMAT_B8G8R8A8_UNORM, width, height, 1, 1);
	assert(SUCCEEDED(hr_));

	const DirectX::Image* img = image.GetImage(0, 0, 0);
	assert(img != nullptr);
	std::memcpy(img->pixels, data, static_cast<size_t>(width) * height * 4);

	DirectX::ScratchImage mipImages{};
	hr_ = DirectX::GenerateMipMaps(image.GetImages(), image.GetImageCount(), image.GetMetadata(), DirectX::TEX_FILTER_SRGB, 0, mipImages);
	assert(SUCCEEDED(hr_));

	TextureData& textureData = textureDatas[key];
	textureData.filePath = key;
	textureData.metadata = mipImages.GetMetadata();
	textureData.resource = CreateTextureResource(textureData.metadata);

	UploadTextureData(textureData.resource.Get(), mipImages);

	textureData.srvIndex = srvManager_->Allocate();
	textureData.srvHandleCPU = srvManager_->GetCPUDescriptorHandle(textureData.srvIndex);
	textureData.srvHandleGPU = srvManager_->GetGPUDescriptorHandle(textureData.srvIndex);

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = textureData.metadata.format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = UINT(textureData.metadata.mipLevels);

	dxCommon_->GetDevice()->CreateShaderResourceView(textureData.resource.Get(), &srvDesc, textureData.srvHandleCPU);

	std::string log = "Embedded Texture Loaded (RGBA8): " + key + " | SRV Index: " + std::to_string(textureData.srvIndex) + " | GPU Handle: " + std::to_string(textureData.srvHandleGPU.ptr) + "\n";
	OutputDebugStringA(log.c_str());
}
void TextureManager::RegisterExternalTexture(const std::string& key, ID3D12Resource* resource, uint32_t srvIndex, DXGI_FORMAT format, uint32_t width, uint32_t height, uint32_t mipLevels) {
	if (key.empty()) {
		return;
	}

	TextureData& textureData = textureDatas[key];
	textureData.filePath = key;
	textureData.resource = resource;
	textureData.srvIndex = srvIndex;
	textureData.srvHandleCPU = srvManager_->GetCPUDescriptorHandle(srvIndex);
	textureData.srvHandleGPU = srvManager_->GetGPUDescriptorHandle(srvIndex);

	textureData.metadata = {};
	textureData.metadata.width = width;
	textureData.metadata.height = height;
	textureData.metadata.depth = 1;
	textureData.metadata.arraySize = 1;
	textureData.metadata.mipLevels = mipLevels;
	textureData.metadata.format = format;
	textureData.metadata.dimension = DirectX::TEX_DIMENSION_TEXTURE2D;
	textureData.metadata.miscFlags = 0;
	textureData.metadata.miscFlags2 = 0;
}
// メタデータに基づいてテクスチャリソースを作成する
Microsoft::WRL::ComPtr<ID3D12Resource> TextureManager::CreateTextureResource(DirectX::TexMetadata& metadata) {

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
	Microsoft::WRL::ComPtr<ID3D12Resource> textureResource_ = nullptr;
	HRESULT hr_ = dxCommon_->GetDevice()->CreateCommittedResource(
	    &heapProperties,                   // Heapの設定
	    D3D12_HEAP_FLAG_NONE,              // Heapの特殊な設定。特になし。
	    &resourceDesc,                     // Resourceの設定
	    D3D12_RESOURCE_STATE_GENERIC_READ, // 初回のResourceState。Textureは基本読むだけ
	    nullptr,                           // Clear最適値。使わないのでnullptr
	    IID_PPV_ARGS(&textureResource_));  // 作成するResourceポインタへのポインタ
	assert(SUCCEEDED(hr_));
	return textureResource_;
}

// uint32_t TextureManager::GetTextureIndexByfilePath(const std::string& filePath){
//
//	LoadTextureName(filePath);
//
//
//	auto it = std::find_if(textureDatas.begin(), textureDatas.end(), [&](const auto& pair) { return pair.second.filePath == filePath; });
//
//
//	if (it != textureDatas.end()) {
//
//		uint32_t textureIndex = static_cast<uint32_t>(std::distance(textureDatas.begin(), it));
//		return textureIndex;
//	}
//	assert(0);
//	return 0;
// }
// ファイルパスから SRV インデックスを取得する
uint32_t TextureManager::GetTextureIndexByfilePath(const std::string& filePath) {
	LoadTextureName(filePath);

	auto it = textureDatas.find(filePath);
	assert(it != textureDatas.end());

	return it->second.srvIndex;
}

uint32_t TextureManager::GetTextureIndexByfilePathAndRefreshTexture(const std::string& filePath)
{
	RefreshTexture(filePath);
	auto it = textureDatas.find(filePath);
	assert(it != textureDatas.end());

	return it->second.srvIndex;
}


// ファイルパスから SRV インデックスを取得する
uint32_t TextureManager::GetsrvIndex(const std::string& filePath) {
	// 未読み込みなら読み込む
	LoadTextureName(filePath);

	auto it = textureDatas.find(filePath);
	assert(it != textureDatas.end());

	return it->second.srvIndex;
}

// ファイルパスから GPU SRV ハンドルを取得する
D3D12_GPU_DESCRIPTOR_HANDLE TextureManager::GetSrvHandleGPU(const std::string& filePath) {
	LoadTextureName(filePath);

	auto it = textureDatas.find(filePath);
	assert(it != textureDatas.end());

	return it->second.srvHandleGPU;
}

// ファイルパスからテクスチャのメタデータを取得する
const DirectX::TexMetadata& TextureManager::GetMetaData(const std::string& filePath) {
	LoadTextureName(filePath);

	auto it = textureDatas.find(filePath);
	assert(it != textureDatas.end());

	return it->second.metadata;
}

// 全 mip レベルの画像データを GPU リソースへ書き込む
void TextureManager::UploadTextureData(ID3D12Resource* texture, const DirectX::ScratchImage& mipImages) {
	// Meta情報を取得
	const DirectX::TexMetadata& metadata = mipImages.GetMetadata();

	for (size_t arrayIndex = 0; arrayIndex < metadata.arraySize; ++arrayIndex) {
		for (size_t mipLevel = 0; mipLevel < metadata.mipLevels; ++mipLevel) {
			const DirectX::Image* img = mipImages.GetImage(mipLevel, arrayIndex, 0);
			assert(img != nullptr);

			const UINT subresourceIndex = static_cast<UINT>(mipLevel + arrayIndex * metadata.mipLevels);
			HRESULT hr_ = texture->WriteToSubresource(subresourceIndex, nullptr, img->pixels, UINT(img->rowPitch), UINT(img->slicePitch));
			assert(SUCCEEDED(hr_));
		}
	}
} // SRV インデックスからテクスチャメタデータを取得する
DirectX::TexMetadata& TextureManager::GetMetaData(uint32_t srvIndex) {
	for (auto& [key, data] : textureDatas) {
		if (data.srvIndex == srvIndex) {
			return data.metadata;
		}
	}

	// RenderTexture など TextureManager の map 管理外の SRV が来るケースでは
	// 参照可能なダミー値を返してアサート終了を避ける。
	static DirectX::TexMetadata dummy{};
	dummy.width = 1;
	dummy.height = 1;
	dummy.mipLevels = 1;

	std::string log = "[TextureManager] Warning: metadata not found for srvIndex=" + std::to_string(srvIndex) + "\n";
	OutputDebugStringA(log.c_str());
	return dummy;
}
// SRV インデックスから GPU SRV ハンドルを取得する
D3D12_GPU_DESCRIPTOR_HANDLE TextureManager::GetSrvHandleGPU(uint32_t srvIndex) {
	for (auto& [key, data] : textureDatas) {
		if (data.srvIndex == srvIndex) {
			return data.srvHandleGPU;
		}
	}
	// RenderTexture など TextureManager で管理していない SRV は
	// SRV マネージャーから直接ハンドルを取得する。
	assert(srvManager_ != nullptr);
	return srvManager_->GetGPUDescriptorHandle(srvIndex);

}

void TextureManager::RefreshTexture(const std::string& filePath) {
	// 1. すでに読み込まれているか確認
	auto it = textureDatas.find(filePath);
	if (it == textureDatas.end()) {
		// まだ読み込まれていない場合は通常のロードを行う
		LoadTextureName(filePath);
		return;
	}

	TextureData& textureData = it->second;

	// 2. 新しい画像を読み込む
	DirectX::ScratchImage image{};
	std::wstring filePathW = StringUtility::ConvertString_(filePath);
	const bool isDDS = filePathW.size() >= 4 && _wcsicmp(filePathW.c_str() + (filePathW.size() - 4), L".dds") == 0;
	HRESULT hr = S_FALSE;
	if (isDDS) {
		hr = DirectX::LoadFromDDSFile(filePathW.c_str(), DirectX::DDS_FLAGS_NONE, nullptr, image);
	} else {
		hr = DirectX::LoadFromWICFile(filePathW.c_str(), DirectX::WIC_FLAGS_FORCE_SRGB, nullptr, image);
	}
	if (FAILED(hr)) {
		return; // ファイルがまだ書き込まれていない等の場合はスキップ
	}

	// ミップマップの作成
	DirectX::ScratchImage mipImages{};
	if (DirectX::IsCompressed(image.GetMetadata().format)) {
		mipImages = std::move(image);
	} else {
		hr = DirectX::GenerateMipMaps(image.GetImages(), image.GetImageCount(), image.GetMetadata(), DirectX::TEX_FILTER_SRGB, 0, mipImages);
		assert(SUCCEEDED(hr));
	}

	// 3. 新しいメタデータでリソースを作り直す
	// (※サイズが変わる可能性があるため、リソース自体は作り直すのが安全です)
	textureData.metadata = mipImages.GetMetadata();
	if (isDDS && textureData.metadata.dimension == DirectX::TEX_DIMENSION_TEXTURE2D && textureData.metadata.arraySize >= 6 && (textureData.metadata.arraySize % 6) == 0 &&
	    textureData.metadata.depth == 1) {
		textureData.metadata.miscFlags |= DirectX::TEX_MISC_TEXTURECUBE;
	}
	textureData.resource = CreateTextureResource(textureData.metadata);

	// 画像データをアップロード
	UploadTextureData(textureData.resource.Get(), mipImages);

	// 4. 【重要】既存の srvHandleCPU を使い、新しいリソースで SRV を作成（上書き）
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = textureData.metadata.format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	if (textureData.metadata.IsCubemap()) {
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
		srvDesc.TextureCube.MostDetailedMip = 0;
		srvDesc.TextureCube.MipLevels = UINT(textureData.metadata.mipLevels);
		srvDesc.TextureCube.ResourceMinLODClamp = 0.0f;
	} else {
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = UINT(textureData.metadata.mipLevels);
	}

	// 既存のハンドルに対して CreateShaderResourceView を呼ぶことで中身が差し替わる
	dxCommon_->GetDevice()->CreateShaderResourceView(textureData.resource.Get(), &srvDesc, textureData.srvHandleCPU);

	// デバッグログ
	std::string log = "[TextureManager] Refreshed: " + filePath + " (Index: " + std::to_string(textureData.srvIndex) + ")\n";
	OutputDebugStringA(log.c_str());
}