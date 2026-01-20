#pragma once
#include <map>
#include <string>
#include <memory>

class Model;
class ModelCommon;
class DirectXCommon;

class ModelManager {

	static std::unique_ptr<ModelManager> instance;


	ModelManager(ModelManager&) = delete;
	ModelManager& operator=(ModelManager&) = delete;

	std::map<std::string, std::unique_ptr<Model>> models;
	std::unique_ptr<ModelCommon> modelCommon_ = nullptr;

	public:

		ModelManager() = default;
	    ~ModelManager() = default;
		static ModelManager* GetInstance();

		void Initialize(DirectXCommon* dxCommon);
	    void LoadModel(const std::string& directionalPath, const std::string& filePath);
	    void LoadGltfModel(const std::string& directionalPath,const std::string& filePath);
	    Model* FindModel(const std::string& filePath);
		void Finalize();


		ModelCommon* GetModelCommon() const { return modelCommon_.get(); }
};
