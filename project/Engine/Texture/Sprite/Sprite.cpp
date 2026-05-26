#define NOMINMAX
#include "Sprite.h"
#include "DirectXCommon.h"
#include "Function.h"
#include "SpriteCommon.h"
#include "SrvManager/SrvManager.h"
#include "TextureManager.h"
#include <algorithm>
void Sprite::Initialize(uint32_t Handle) {

	textureIndex = Handle;

	vertexResource = SpriteCommon::GetInstance()->CreateBufferResource(sizeof(VertexData) * kMaxSpriteVertices);
	vertexBufferView.BufferLocation = vertexResource->GetGPUVirtualAddress();
	vertexBufferView.SizeInBytes = sizeof(VertexData) * kMaxSpriteVertices;
	vertexBufferView.StrideInBytes = sizeof(VertexData);

	// 頂点データ

	vertexResource->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
	// ……スプライトの頂点6つ設定……
	// 頂点は 4つだけにする
	// 左上
	vertexData[0].position = {0.0f, 0.0f, 0.0f, 1.0f};
	vertexData[0].texcoord = {0.0f, 0.0f};

	// 右上
	vertexData[1].position = {1.0f, 0.0f, 0.0f, 1.0f};
	vertexData[1].texcoord = {1.0f, 0.0f};

	// 左下
	vertexData[2].position = {0.0f, 1.0f, 0.0f, 1.0f};
	vertexData[2].texcoord = {0.0f, 1.0f};

	// 右下
	vertexData[3].position = {1.0f, 1.0f, 0.0f, 1.0f};
	vertexData[3].texcoord = {1.0f, 1.0f};

	// 頂点は4つだけ
	for (int i = 0; i < 4; i++) {
		vertexData[i].normal = {0.0f, 0.0f, -1.0f};
	}

	vertexResource->Unmap(0, nullptr);

	// インデックスデータ
	uint32_t indices[6] = {0, 1, 2, 2, 1, 3};
	indexResource = SpriteCommon::GetInstance()->CreateBufferResource(sizeof(uint32_t) * 6);
	void* mapped = nullptr;
	indexResource->Map(0, nullptr, &mapped);
	memcpy(mapped, indices, sizeof(indices));
	indexResource->Unmap(0, nullptr);

	indexBufferView.BufferLocation = indexResource->GetGPUVirtualAddress();
	indexBufferView.SizeInBytes = sizeof(indices);
	indexBufferView.Format = DXGI_FORMAT_R32_UINT;

	size_t alignedSize = (sizeof(Material) + 0xFF) & ~0xFF;
	// スプライト用（陰影つけたくないもの）
	materialResource = SpriteCommon::GetInstance()->CreateBufferResource(alignedSize);
	Material* matSprite = nullptr;
	materialResource->Map(0, nullptr, reinterpret_cast<void**>(&matSprite));
	matSprite->color = Vector4(1.0f, 1.0f, 1.0f, 1.0f); // 白 or テクスチャの色
	matSprite->enableLighting = false;
	matSprite->uvTransform = Function::MakeIdentity4x4();
	material = matSprite;
	materialResource->Unmap(0, nullptr);

	transformResource = SpriteCommon::GetInstance()->CreateBufferResource(sizeof(TransformationMatrix));
	transformResource->Map(0, nullptr, reinterpret_cast<void**>(&transformData));
	transformData->WVP = Function::MakeIdentity4x4();
	transformData->World = Function::MakeIdentity4x4();

	AdjustTextureSize();
	SetTextureRange({0.0f, 0.0f}, textureSize);
}

void Sprite::Draw() {
	const SpriteCommon* spriteCommon = SpriteCommon::GetInstance();
	if (spriteCommon->GetDxCommon()->IsEditorLayoutEnabled() && !spriteCommon->IsSpriteVisible()) {
		return;
	}

	// 頂点バッファビューとインデックスバッファビューをセット（オフセット指定）
	UINT offset = currentSpriteVertexOffset_;
	D3D12_VERTEX_BUFFER_VIEW vbv = vertexBufferView;
	vbv.BufferLocation += sizeof(VertexData) * offset;
	vbv.SizeInBytes = sizeof(VertexData) * 4; // 4頂点分

	D3D12_INDEX_BUFFER_VIEW ibv = indexBufferView;
	// インデックスバッファは使いまわしでOK（インデックス: 0,1,2, 0,2,3 など4頂点分用を用意）

	SpriteCommon::GetInstance()->GetDxCommon()->GetCommandList()->IASetVertexBuffers(0, 1, &vbv);
	SpriteCommon::GetInstance()->GetDxCommon()->GetCommandList()->IASetIndexBuffer(&ibv);

	// ヒープ、ルートパラメータ等をセット（すでにやってる場合は不要）
	ID3D12DescriptorHeap* heaps[] = {TextureManager::GetInstance()->GetSrvManager()->GetDescriptorHeap().Get()};
	SpriteCommon::GetInstance()->GetDxCommon()->GetCommandList()->SetDescriptorHeaps(_countof(heaps), heaps);
	SpriteCommon::GetInstance()->GetDxCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(0, materialResource->GetGPUVirtualAddress());
	SpriteCommon::GetInstance()->GetDxCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(1, transformResource->GetGPUVirtualAddress());
	SpriteCommon::GetInstance()->GetDxCommon()->GetCommandList()->SetGraphicsRootDescriptorTable(2, TextureManager::GetInstance()->GetSrvHandleGPU(textureIndex));

	// 描画（1スプライト分）
	SpriteCommon::GetInstance()->GetDxCommon()->GetCommandList()->DrawIndexedInstanced(6, 1, 0, 0, 0);

	// 次のスプライト用にオフセットを進める
}

void Sprite::Update() {

	if (isFlipX_ || isFripY_) {
		if (isFlipX_) {
			left = -left;
			right = -right;
		}
		if (isFripY_) {
			top = -top;
			bottom = -bottom;
		}
	}
	float localLeft = left;
	float localRight = right;
	float localTop = top;
	float localBottom = bottom;
	float uLeft = uvLeftTop_.x;
	float uRight = uvRightBottom_.x;
	float vTop = uvLeftTop_.y;
	float vBottom = uvRightBottom_.y;

	if (enableClip_) {
		float worldLeft = transform_.translate.x + localLeft * transform_.scale.x;
		float worldRight = transform_.translate.x + localRight * transform_.scale.x;
		float worldTop = transform_.translate.y + localTop * transform_.scale.y;
		float worldBottom = transform_.translate.y + localBottom * transform_.scale.y;

		float clippedLeft = std::max(worldLeft, clipRect_.x);
		float clippedRight = std::min(worldRight, clipRect_.z);
		float clippedTop = std::max(worldTop, clipRect_.y);
		float clippedBottom = std::min(worldBottom, clipRect_.w);

		if (clippedLeft >= clippedRight || clippedTop >= clippedBottom) {
			material->color.w = 0.0f;
		} else {
			material->color.w = 1.0f;
			float width = (worldRight - worldLeft);
			float height = (worldBottom - worldTop);
			float leftRate = (clippedLeft - worldLeft) / width;
			float rightRate = (clippedRight - worldLeft) / width;
			float topRate = (clippedTop - worldTop) / height;
			float bottomRate = (clippedBottom - worldTop) / height;

			localLeft = left + (right - left) * leftRate;
			localRight = left + (right - left) * rightRate;
			localTop = top + (bottom - top) * topRate;
			localBottom = top + (bottom - top) * bottomRate;

			float du = (uvRightBottom_.x - uvLeftTop_.x);
			float dv = (uvRightBottom_.y - uvLeftTop_.y);
			uLeft = uvLeftTop_.x + du * leftRate;
			uRight = uvLeftTop_.x + du * rightRate;
			vTop = uvLeftTop_.y + dv * topRate;
			vBottom = uvLeftTop_.y + dv * bottomRate;
		}
	}

	vertexData[0].position = {localLeft, localTop, 0.0f, 1.0f};
	vertexData[1].position = {localRight, localTop, 0.0f, 1.0f};
	vertexData[2].position = {localLeft, localBottom, 0.0f, 1.0f};
	vertexData[3].position = {localRight, localBottom, 0.0f, 1.0f};
	vertexData[0].texcoord = {uLeft, vTop};
	vertexData[1].texcoord = {uRight, vTop};
	vertexData[2].texcoord = {uLeft, vBottom};
	vertexData[3].texcoord = {uRight, vBottom};

	// World
	Matrix4x4 world = Function::MakeAffineMatrix(transform_.scale, transform_.rotate, transform_.translate);

	// 画面サイズに基づく正射影。left=0, top=0, right=幅, bottom=高さ, near=0, far=1
	Matrix4x4 ortho = Function::MakeOrthographicMatrix(0.0f, 0.0f, static_cast<float>(WinApp::kClientWidth), static_cast<float>(WinApp::kClientHeight), 0.0f, 1.0f);

	// 行列の掛け順は VS 側の mul(input.position, WVP) に合わせる
	Matrix4x4 wvp = Function::Multiply(world, ortho);

	transformData->WVP = wvp;
	transformData->World = world;
}

void Sprite::SetColor(const Vector4& color) { material->color = color; };

void Sprite::SetTextureRange(const Vector2& leftTop, const Vector2& TextureSize) {

	const DirectX::TexMetadata& metaData = TextureManager::GetInstance()->GetMetaData(textureIndex);
	float textureWidth = static_cast<float>(metaData.width);
	float textureHeight = static_cast<float>(metaData.height);
	if (textureWidth <= 0.0f || textureHeight <= 0.0f) {
		textureWidth = 1.0f;
		textureHeight = 1.0f;
	}
	float tex_left = leftTop.x / textureWidth;
	float tex_right = (leftTop.x + TextureSize.x) / textureWidth;
	float tex_top = leftTop.y / textureHeight;
	float tex_bottom = (leftTop.y + TextureSize.y) / textureHeight;

	uvLeftTop_ = {tex_left, tex_top};
	uvRightBottom_ = {tex_right, tex_bottom};
	vertexData[0].texcoord = {tex_left, tex_top};
	vertexData[1].texcoord = {tex_right, tex_top};
	vertexData[2].texcoord = {tex_left, tex_bottom};
	vertexData[3].texcoord = {tex_right, tex_bottom};
}

void Sprite::SetClipRect(const Vector4& clipRect) {
	enableClip_ = true;
	clipRect_ = clipRect;
}

void Sprite::ClearClipRect() { enableClip_ = false; }

void Sprite::SetClipBySprite(const Sprite& sprite) {
	Vector2 lt = sprite.GetLeftTop();
	Vector2 rb = sprite.GetRightBottom();
	SetClipRect({lt.x, lt.y, rb.x, rb.y});
}


void Sprite::SetIsFlipX(const bool isFlipX) { isFlipX_ = isFlipX; }

void Sprite::SetIsFlipY(const bool isFlipY) { isFripY_ = isFlipY; }

void Sprite::AdjustTextureSize() {
	DirectX::TexMetadata& metadata = TextureManager::GetInstance()->GetMetaData(textureIndex);

	if (metadata.width == 0 || metadata.height == 0) {
		textureCutSize = {1.0f, 1.0f};
	} else {
		textureCutSize.x = static_cast<float>(metadata.width);
		textureCutSize.y = static_cast<float>(metadata.height);
	}

	textureSize = textureCutSize;
}
