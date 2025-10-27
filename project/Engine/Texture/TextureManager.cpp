#include "TextureManager.h"
#include "DirectXCommon.h"
#include "StringUtility.h"
TextureManager* TextureManager::instance = nullptr;
uint32_t TextureManager::kSRVIndexTop = 1;

TextureManager* TextureManager::GetInstance(){

	if (instance == nullptr) {
		instance = new TextureManager;
	
	}
	return instance;

}

void TextureManager::Initialize(DirectXCommon* dxCommon){

	textureDatas.reserve(DirectXCommon::kMaxSRVCount);
	
	dxCommon_ = dxCommon;
}

void TextureManager::Finalize(){ 
	delete instance;
	instance = nullptr;
}

void TextureManager::LoadTextureName(const std::string& filePath) {

	auto it = std::find_if(textureDatas.begin(), textureDatas.end(), [&](TextureData& textureData) { return textureData.filePath == filePath; });
	if (it != textureDatas.end()) {
		return;
	}
	assert(textureDatas.size() + kSRVIndexTop <= DirectXCommon::kMaxSRVCount);

	// テクスチャファイルを読んでプログラムで使えるようにする
	DirectX::ScratchImage image{};
	std::wstring filePathW = StringUtility::ConvertString_(filePath);
	HRESULT hr_ = DirectX::LoadFromWICFile(filePathW.c_str(), DirectX::WIC_FLAGS_FORCE_SRGB, nullptr, image);
	assert(SUCCEEDED(hr_));

	// ミップマップの作成
	DirectX::ScratchImage mipImages{};
	hr_ = DirectX::GenerateMipMaps(image.GetImages(), image.GetImageCount(), image.GetMetadata(), DirectX::TEX_FILTER_SRGB, 0, mipImages);
	assert(SUCCEEDED(hr_));

	textureDatas.resize(textureDatas.size() + 1);
	TextureData& textureData = textureDatas.back();
	textureData.filePath = filePath;
	textureData.metadata = mipImages.GetMetadata();
	textureData.resource = CreateTextureResource(textureData.metadata);

	// ★ ここを UploadTextureData に統一
	UploadTextureData(textureData.resource.Get(), mipImages);

	// SRVハンドル設定
	uint32_t srvIndex = static_cast<uint32_t>(textureDatas.size() - 1) + kSRVIndexTop;
	textureData.srvHandleCPU = dxCommon_->GetSrvCpuDescriptorHandle(srvIndex);
	textureData.srvHandleGPU = dxCommon_->GetSrvGpuDescriptorHandle(srvIndex);

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = textureData.metadata.format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = UINT(textureData.metadata.mipLevels);

	dxCommon_->GetDevice()->CreateShaderResourceView(textureData.resource.Get(), &srvDesc, textureData.srvHandleCPU);

	// デバッグログ出力
	std::string log = "Texture Loaded: " + filePath + " | SRV Index: " + std::to_string(srvIndex) + " | GPU Handle: " + std::to_string(textureData.srvHandleGPU.ptr) + "\n";
	OutputDebugStringA(log.c_str());

}

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

uint32_t TextureManager::GetTextureIndexByfilePath(const std::string& filePath){

	LoadTextureName(filePath);


	auto it = std::find_if(textureDatas.begin(), textureDatas.end(), [&](TextureData& textureData) { return textureData.filePath == filePath; });

	if (it != textureDatas.end()) {
	
		uint32_t textureIndex = static_cast<uint32_t>(std::distance(textureDatas.begin(), it));
		return textureIndex;
	}
	assert(0);
	return 0;
}
D3D12_GPU_DESCRIPTOR_HANDLE TextureManager::GetSrvHandleGPU(uint32_t textureIndex) {
	// 範囲外指定違反チェック
	assert(textureIndex < textureDatas.size());

	// テクスチャデータの参照を取得
	TextureData& textureData = textureDatas[textureIndex];

	return textureData.srvHandleGPU;
}
DirectX::TexMetadata& TextureManager::GetMetaData(uint32_t textureIndex) {
	// 範囲外アクセス防止
	assert(textureIndex < textureDatas.size());

	// 対応するテクスチャデータを返す
	return textureDatas[textureIndex].metadata;
}

void TextureManager::UploadTextureData(ID3D12Resource* texture, const DirectX::ScratchImage& mipImages) {
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