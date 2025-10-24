#include "Sprite.h"
#include "SpriteCommon.h"
#include "Function.h"
#include "DirectXCommon.h"
#include "TextureManager.h"

void Sprite::Initialize(SpriteCommon* spriteCommon,std::string textureFilePath){

	spriteCommon_ = spriteCommon;

	 TextureManager::GetInstance()->LoadTextureName(textureFilePath);

	textureIndex = TextureManager::GetInstance()->GetTextureIndexByfilePath(textureFilePath);

	vertexResource = spriteCommon_->CreateBufferResource(sizeof(VertexData) * kMaxSpriteVertices);
	vertexBufferView.BufferLocation = vertexResource->GetGPUVirtualAddress();
	vertexBufferView.SizeInBytes = sizeof(VertexData) * kMaxSpriteVertices;
	vertexBufferView.StrideInBytes = sizeof(VertexData);

	// 頂点データ
	
	vertexResource->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
	// ……スプライトの頂点6つ設定……
	vertexData[0].position = {0.0f, 360.0f, 0.0f, 1.0f}; // 左下
	vertexData[0].texcoord = {0.0f, 1.0f};

	vertexData[1].position = {0.0f, 0.0f, 0.0f, 1.0f}; // 左上
	vertexData[1].texcoord = {0.0f, 0.0f};

	vertexData[2].position = {640.0f, 360.0f, 0.0f, 1.0f}; // 右下
	vertexData[2].texcoord = {1.0f, 1.0f};

	// 2枚目の三角形（左上 → 右上 → 右下）
	vertexData[3].position = {0.0f, 0.0f, 0.0f, 1.0f}; // 左上
	vertexData[3].texcoord = {0.0f, 0.0f};

	vertexData[4].position = {640.0f, 0.0f, 0.0f, 1.0f}; // 右上
	vertexData[4].texcoord = {1.0f, 0.0f};

	vertexData[5].position = {640.0f, 360.0f, 0.0f, 1.0f}; // 右下
	vertexData[5].texcoord = {1.0f, 1.0f};

	for (int i = 0; i < 6; i++) {
		vertexData[i].normal = {0.0f, 0.0f, -1.0f};
	}
	vertexResource->Unmap(0, nullptr);

	// インデックスデータ
	uint32_t indices[6] = {0, 1, 2, 0, 2, 3};
	indexResource = spriteCommon_->CreateBufferResource(sizeof(uint32_t) * 6);
	void* mapped = nullptr;
	indexResource->Map(0, nullptr, &mapped);
	memcpy(mapped, indices, sizeof(indices));
	indexResource->Unmap(0, nullptr);

	indexBufferView.BufferLocation = indexResource->GetGPUVirtualAddress();
	indexBufferView.SizeInBytes = sizeof(indices);
	indexBufferView.Format = DXGI_FORMAT_R32_UINT;



	// --- ここにインデックスバッファの生成を追加 ---
	// --- ここにインデックスバッファの生成を追加 ---
	// 6個のインデックス（2枚の三角形でスプライト）
	indexResource = spriteCommon_->CreateBufferResource(sizeof(uint32_t) * 6);

	size_t alignedSize = (sizeof(Material) + 0xFF) & ~0xFF;
	// スプライト用（陰影つけたくないもの）
	materialResource = spriteCommon_->CreateBufferResource(alignedSize);
	Material* matSprite = nullptr;
	materialResource->Map(0, nullptr, reinterpret_cast<void**>(&matSprite));
	matSprite->color = Vector4(1.0f, 1.0f, 1.0f, 1.0f); // 白 or テクスチャの色
	matSprite->enableLighting = 0;
	matSprite->uvTransform = Function::MakeIdentity4x4();
	materialResource->Unmap(0, nullptr);

	// Sprite用の TransformationMatrix リソース作成（1個分）
	transformResource = spriteCommon_->CreateBufferResource(sizeof(Matrix4x4) * 2);

	// データへのポインタ取得
	transformData = nullptr;
	transformResource->Map(0, nullptr, reinterpret_cast<void**>(&transformData));

	// 単位行列を書き込んでおく（初期状態）
	*transformData = Function::MakeIdentity4x4();
	
}

void Sprite::Draw(){

	

	// 頂点バッファビューとインデックスバッファビューをセット（オフセット指定）
	UINT offset = currentSpriteVertexOffset_;
	D3D12_VERTEX_BUFFER_VIEW vbv = vertexBufferView;
	vbv.BufferLocation += sizeof(VertexData) * offset;
	vbv.SizeInBytes = sizeof(VertexData) * 4; // 4頂点分

	D3D12_INDEX_BUFFER_VIEW ibv = indexBufferView;
	// インデックスバッファは使いまわしでOK（インデックス: 0,1,2, 0,2,3 など4頂点分用を用意）

	spriteCommon_->GetDxCommon()->GetCommandList()->IASetVertexBuffers(0, 1, &vbv);
	spriteCommon_->GetDxCommon()->GetCommandList()->IASetIndexBuffer(&ibv);
	spriteCommon_->GetDxCommon()->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// ヒープ、ルートパラメータ等をセット（すでにやってる場合は不要）
	ID3D12DescriptorHeap* heaps[] = {spriteCommon_->GetDxCommon()->GetSrvDescriptorHeap()};
	spriteCommon_->GetDxCommon()->GetCommandList()->SetDescriptorHeaps(_countof(heaps), heaps);
	spriteCommon_->GetDxCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(0, materialResource->GetGPUVirtualAddress());
	spriteCommon_->GetDxCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(1, transformResource->GetGPUVirtualAddress());
	spriteCommon_->GetDxCommon()->GetCommandList()->SetGraphicsRootDescriptorTable(2, TextureManager::GetInstance()->GetSrvHandleGPU(textureIndex));

	// 描画（1スプライト分）
	spriteCommon_->GetDxCommon()->GetCommandList()->DrawIndexedInstanced(6, 1, 0, 0, 0);

	// 次のスプライト用にオフセットを進める
	currentSpriteVertexOffset_ += 4; // 4頂点分進める


}

void Sprite::Update() {
	

	// World行列 = S * R * T
	Matrix4x4 world = Function::MakeAffineMatrix(transform_.scale,transform_.rotate,transform_.translate);

	*transformData = world;
}











