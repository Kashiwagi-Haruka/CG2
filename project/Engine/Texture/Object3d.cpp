#include "Object3d.h"
#include "Object3dCommon.h"
#include "DirectXCommon.h"
#include <fstream>
#include <sstream>
#include <cassert>
#include "Function.h"
#include "TextureManager.h"

void Object3d::Initialize(Object3dCommon* modelCommon){ 
	modelCommon_ = modelCommon;
	CreateResources();
	TextureManager::GetInstance()->LoadTextureName(modelData_.material.textureFilePath);
	modelData_.material.textureIndex = TextureManager::GetInstance()->GetTextureIndexByfilePath(modelData_.material.textureFilePath);
}

void Object3d::Draw() {

	// --- SRVヒープをバインド ---
	ID3D12DescriptorHeap* descriptorHeaps[] = {modelCommon_->GetDxCommon()->GetSrvDescriptorHeap()};
	modelCommon_->GetDxCommon()->GetCommandList()->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);


	// --- VertexBufferViewを設定 ---
	modelCommon_->GetDxCommon()->GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferView_);
	

	// --- マテリアルCBufferの場所を設定 ---
	modelCommon_->GetDxCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(0, materialResource_->GetGPUVirtualAddress());

	// --- 座標変換行列CBufferの場所を設定 ---
	modelCommon_->GetDxCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(1, transformResource_->GetGPUVirtualAddress());

	// --- SRVのDescriptorTableの先頭を設定 ---
	// TextureManagerからSRVのGPUハンドルを取得
	D3D12_GPU_DESCRIPTOR_HANDLE srvHandle = TextureManager::GetInstance()->GetSrvHandleGPU(modelData_.material.textureIndex);
	modelCommon_->GetDxCommon()->GetCommandList()->SetGraphicsRootDescriptorTable(2, srvHandle);

	// --- 平行光源CBufferの場所を設定 ---
	modelCommon_->GetDxCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(3, directionalLightResource_->GetGPUVirtualAddress());

	// --- 描画！（DrawCall）---
	modelCommon_->GetDxCommon()->GetCommandList()->DrawInstanced(
	    static_cast<UINT>(modelData_.vertices.size()), // 頂点数
	    1,                                             // インスタンス数
	    0,                                             // 開始頂点位置
	    0                                              // 開始インスタンス位置
	);
}

// objfileを読む関数
void Object3d::LoadObjFile(const std::string& directoryPath, const std::string& filename) {
	ModelData modelData;
	std::vector<Vector4> positions;
	std::vector<Vector3> normals;
	std::vector<Vector2> texcoords;
	std::string line;

	std::ifstream file(directoryPath + "/" + filename + ".obj");
	assert(file.is_open());

	while (std::getline(file, line)) {
		std::string identifier;
		std::istringstream s(line);
		s >> identifier;

		if (identifier == "v") {
			Vector4 position;
			s >> position.x >> position.y >> position.z;
			position.w = 1.0f;
			// ここでx反転
			position.x *= -1.0f;

			positions.push_back(position);
		} else if (identifier == "vt") {
			Vector2 texcoord;
			s >> texcoord.x >> texcoord.y;
			texcoord.y = 1.0f - texcoord.y;
			texcoords.push_back(texcoord);
		} else if (identifier == "vn") {
			Vector3 normal;
			s >> normal.x >> normal.y >> normal.z;
			// ここで法線もx反転

			normals.push_back(normal);
		} else if (identifier == "f") {
			VertexData triangle[3];
			for (int32_t faceVertex = 0; faceVertex < 3; ++faceVertex) {
				std::string vertexDefinition;
				s >> vertexDefinition;
				std::istringstream v(vertexDefinition);
				uint32_t elementIndices[3];
				for (int32_t element = 0; element < 3; ++element) {
					std::string index;
					std::getline(v, index, '/');
					elementIndices[element] = std::stoi(index);
				}
				Vector4 position = positions[elementIndices[0] - 1];
				Vector2 texcoord = texcoords[elementIndices[1] - 1];
				Vector3 normal = normals[elementIndices[2] - 1];
				triangle[faceVertex] = {position, texcoord, normal};
			}
			// 回り順を逆にしてpush_back
			modelData.vertices.push_back(triangle[2]);
			modelData.vertices.push_back(triangle[1]);
			modelData.vertices.push_back(triangle[0]);
		} else if (identifier == "mtllib") {
			std::string mtlFile;
			s >> mtlFile;
			modelData.material = LoadMaterialTemplateFile(directoryPath, mtlFile);
		}
	}

	modelData_ = modelData;
}

Object3d::MaterialData Object3d::LoadMaterialTemplateFile(const std::string& directoryPath, const std::string& filename) {
	MaterialData matData;
	std::ifstream file(directoryPath + "/" + filename);
	std::string line;
	assert(file.is_open());
	while (std::getline(file, line)) {
		std::istringstream s(line);
		std::string identifier;
		s >> identifier;
		if (identifier == "map_Kd") {
			std::string textureFilePaths;
			s >> textureFilePaths;
			matData.textureFilePath = directoryPath + "/" + textureFilePaths;
		}
	}
	return matData;
}

void Object3d::CreateResources() {
	vertexResource_ = modelCommon_->CreateBufferResource(sizeof(VertexData) * modelData_.vertices.size());
	vertexBufferView_.BufferLocation = vertexResource_->GetGPUVirtualAddress();
	vertexBufferView_.SizeInBytes = UINT(sizeof(VertexData) * modelData_.vertices.size());
	vertexBufferView_.StrideInBytes = sizeof(VertexData);

	VertexData* vertexData = nullptr;
	vertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
	std::memcpy(vertexData, modelData_.vertices.data(), sizeof(VertexData) * modelData_.vertices.size());
	vertexResource_->Unmap(0, nullptr);

	// --- マテリアル用リソース ---
	// 3D用（球など陰影つけたいもの）
	// 必ず256バイト単位で切り上げる
	size_t alignedSize = (sizeof(Material) + 0xFF) & ~0xFF;
	materialResource_ = modelCommon_->CreateBufferResource(alignedSize);
	Material* mat3d = nullptr;
	materialResource_->Map(0, nullptr, reinterpret_cast<void**>(&mat3d));
	mat3d->color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	mat3d->enableLighting = false;
	mat3d->uvTransform = Function::MakeIdentity4x4();

	materialResource_->Unmap(0, nullptr);

	// [0]=モデル描画用で使う
	Matrix4x4 worldMatrix = Function::MakeAffineMatrix(transform_.scale, transform_.rotate, transform_.translate);
	Matrix4x4 cameraMatrix = Function::MakeAffineMatrix(cameraTransform_.scale, cameraTransform_.rotate, cameraTransform_.translate);
	Matrix4x4 viewMatrix = Function::Inverse(cameraMatrix);
	Matrix4x4 projectionMatrix = Function::MakePerspectiveFovMatrix(0.45f, 1280.0f / 720.0f, 0.1f, 100.0f);
	Matrix4x4 worldViewProjectionMatrix = Function::Multiply(worldMatrix, Function::Multiply(viewMatrix, projectionMatrix));
	transformResource_ = modelCommon_->CreateBufferResource(sizeof(TransformationMatrix));
	transformResource_->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrixData_));
	transformationMatrixData_->WVP = worldViewProjectionMatrix;
	transformationMatrixData_->World = worldMatrix;

	// Lightバッファ
	directionalLightResource_ = modelCommon_->CreateBufferResource(sizeof(DirectionalLight));
	assert(directionalLightResource_);
	directionalLightData_ = nullptr;
	directionalLightResource_->Map(0, nullptr, reinterpret_cast<void**>(&directionalLightData_));
	assert(directionalLightData_);
	*directionalLightData_ = {
	    {1.0f, 1.0f, 1.0f, 1.0f},
        {0.0f, -1.0f, 0.0f},
        1.0f
    };
}