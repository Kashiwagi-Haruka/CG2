#pragma once
#include <map>
#include <string>
#include <memory>

class Model;
class ModelCommon;
class DirectXCommon;

class ModelManeger {

	static std::unique_ptr<ModelManeger> instance;


	ModelManeger(ModelManeger&) = delete;
	ModelManeger& operator=(ModelManeger&) = delete;

	std::map<std::string, std::unique_ptr<Model>> models;
	std::unique_ptr<ModelCommon> modelCommon_ = nullptr;

	public:

		ModelManeger() = default;
	    ~ModelManeger() = default;
		static ModelManeger* GetInstance();

		void Initialize(DirectXCommon* dxCommon);
	    void LoadModel(const std::string& filePath);
	    void LoadGltfModel(const std::string& filePath);
	    Model* FindModel(const std::string& filePath);
		void Finalize();



};
