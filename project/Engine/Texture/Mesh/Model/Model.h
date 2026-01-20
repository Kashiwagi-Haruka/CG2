#pragma once
#include "Matrix4x4.h"
#include "VertexData.h"
#include "QuaternionTransform.h"
#include "SkinningData.h"
#include <Windows.h>
#include <d3d12.h>
#include <string>
#include <vector>
#include <wrl.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <map>

class ModelCommon;
struct SkinCluster;
class Model {
public:
	struct Node {
		QuaternionTransform transform;
		Matrix4x4 localMatrix;
		std::string name;
		std::vector<Node> children;
	};

private:
	struct MaterialData {
		std::string textureFilePath;
		uint32_t textureIndex = 0;
	};
	struct ModelData {
		std::map<std::string, JointWeightData> skinClusterData;
		std::vector<VertexData> vertices;
		std::vector<uint32_t> indices;
		MaterialData material;
		Node rootnode;
	};
	struct Material {
		
		Vector4 color;
		int enableLighting;
		float padding[3];
		Matrix4x4 uvTransform;
		float shininess;
		float environmentCoefficient;
		float padding2[2];
	};

	ModelCommon* modelCommon_;

	ModelData modelData_;

	D3D12_VERTEX_BUFFER_VIEW vertexBufferView_;
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource_;
	D3D12_INDEX_BUFFER_VIEW indexBufferView_;
	Microsoft::WRL::ComPtr<ID3D12Resource> indexResource_;

	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource_;
	VertexData* vertexData = nullptr;
	Material* mat3d = nullptr;
	MaterialData LoadMaterialTemplateFile(const std::string& directoryPath, const std::string& filename);
	Node NodeRead(aiNode* node);

public:
	void Initialize(ModelCommon* modelCommon);
	void LoadObjFile(const std::string& directoryPath, const std::string& filename);
	void LoadObjFileAssimp(const std::string& directoryPath, const std::string& filename);
	void LoadObjFileGltf(const std::string& directoryPath, const std::string& filename);
	void Draw();
	void Draw(const SkinCluster* skinCluster);
	void SetColor(Vector4 color);
	void SetEnableLighting(bool enable);
	void SetUvTransform(const Matrix4x4& uvTransform);
	void SetShininess(float shininess);
	void SetEnvironmentCoefficient(float coefficient);
	D3D12_VERTEX_BUFFER_VIEW GetVertexBufferView() { return vertexBufferView_; }
	Microsoft::WRL::ComPtr<ID3D12Resource> GetMaterialResource() { return materialResource_; }
	const ModelData& GetModelData() const { return modelData_; }
};