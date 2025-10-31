#pragma once
#include <string>
#include <vector>
#include "VertexData.h"
#include <Windows.h>
#include <wrl.h>
#include <d3d12.h>
#include "Matrix4x4.h"
#include "Vector4.h"


class Object3dCommon;

class Object3d {

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
	struct Transform {

		Vector3 scale;
		Vector3 rotate;
		Vector3 translate;
	};

	struct alignas(256) TransformationMatrix {
		Matrix4x4 WVP;   // 64 バイト
		Matrix4x4 World; // 64 バイト
		                 // ここで自動的に 128 バイト分のパディングが入って、
		                 // sizeof(TransformationMatrix) == 256 になる
	};

	Transform transform_ = {
	{1.0f,1.0f,1.0f},
	{0.0f,0.0f,0.0f},
	{0.0f,0.0f,0.0f},
	};
	Transform cameraTransform_ = {
	    {1.0f, 1.0f, 1.0f  }, // スケール
	    {0.3f, 0.0f, 0.0f  }, //
	    {0.0f, 4.0f, -10.0f}  //
	};
	struct DirectionalLight {
		Vector4 color;
		Vector3 direction;
		float intensity;
	};


	int color = 0xffffffff;




	Object3dCommon* modelCommon_;
	ModelData modelData_;
	DirectionalLight* directionalLightData_ = nullptr;
	VertexData* vertexData = nullptr;
	TransformationMatrix* transformationMatrixData_;
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView_;
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource_;
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource_;
	Microsoft::WRL::ComPtr<ID3D12Resource> directionalLightResource_ = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> transformResource_;
	Microsoft::WRL::ComPtr<ID3D12Resource> transformationMatrixResource_;


	MaterialData LoadMaterialTemplateFile(const std::string& directoryPath, const std::string& filename);

	public:

	void Initialize(Object3dCommon* modelCommon);
	void Draw();
	void LoadObjFile(const std::string& directoryPath, const std::string& filename);
	void CreateResources();

};
