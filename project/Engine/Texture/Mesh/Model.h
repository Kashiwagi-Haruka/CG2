#pragma once
#include <string>
#include <vector>
#include <Windows.h>
#include <wrl.h>
#include <d3d12.h>
#include "VertexData.h"
#include "Matrix4x4.h"                
class ModelCommon;

class Model {

	struct MaterialData {
		std::string textureFilePath;
		uint32_t textureIndex = 0;
	};
	struct ModelData {
		std::vector<VertexData> vertices;
		MaterialData material;
	};
	struct Material {

		Vector4 color;
		int enableLighting;
		float padding[3];
		Matrix4x4 uvTransform;
	};

	ModelCommon* modelCommon_;

	ModelData modelData_;

	D3D12_VERTEX_BUFFER_VIEW vertexBufferView_;
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource_;

	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource_;
	VertexData* vertexData = nullptr;

	MaterialData LoadMaterialTemplateFile(const std::string& directoryPath, const std::string& filename);

	public:
	void Initialize(ModelCommon* modelCommon);
	void LoadObjFile(const std::string& directoryPath, const std::string& filename);
	void Draw();
	D3D12_VERTEX_BUFFER_VIEW GetVertexBufferView() { return vertexBufferView_; }
	Microsoft::WRL::ComPtr<ID3D12Resource> GetMaterialResource() { return materialResource_; }
	ModelData& GetModelData() { return modelData_; }
	};
