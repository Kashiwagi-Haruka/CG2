#pragma once
#include <map>
#include <string>
#include <memory>

class Model;
class ModelCommon;
class DirectXCommon;

class ModelManeger {

	static ModelManeger* instance;

	ModelManeger() = default;
	~ModelManeger() = default;
	ModelManeger(ModelManeger&) = delete;
	ModelManeger& operator=(ModelManeger&) = delete;

	std::map<std::string, std::unique_ptr<Model>> models;
	ModelCommon* modelCommon_ = nullptr;

	public:

		static ModelManeger* GetInstance();

		void Initialize(DirectXCommon* dxCommon);
	    void LoadModel(const std::string& filePath);
	    Model* FindModel(const std::string& filePath);
		void Finalize();



};
