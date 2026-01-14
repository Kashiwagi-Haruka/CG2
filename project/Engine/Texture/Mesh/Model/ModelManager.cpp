#include "Model/ModelManager.h"
#include "Model/ModelCommon.h"
#include "Model/Model.h"
#include <filesystem>
#include <vector>
std::unique_ptr<ModelManager> ModelManager::instance = nullptr;

ModelManager* ModelManager::GetInstance(){
	if (instance == nullptr) {
		instance = std::make_unique<ModelManager>();
	}
	return instance.get();
}

void ModelManager::Initialize(DirectXCommon* dxCommon){
	modelCommon_ = std::make_unique<ModelCommon>();
	modelCommon_->Initialize(dxCommon);
}

void ModelManager::LoadModel(const std::string& directionalPath, const std::string& filePath) {

	if (models.contains(filePath)) {
		return;
	}

	std::unique_ptr<Model> model = std::make_unique<Model>();
	model->LoadObjFileAssimp(directionalPath, filePath + ".obj");
	model->Initialize(modelCommon_.get());
	models.insert(std::make_pair(filePath, std::move(model)));
}
void ModelManager::LoadGltfModel(const std::string& directionalPath , const std::string& filePath) {
	if (models.contains(filePath)) {
		return;
	}
	namespace fs = std::filesystem;
	const fs::path basePath = fs::path(directionalPath) / filePath;
	std::vector<fs::path> candidates;
	fs::path directoryPath = directionalPath;
	std::string filename = filePath;

	if (basePath.has_extension()) {
		candidates.push_back(basePath);
		if (basePath.has_parent_path()) {
			directoryPath = basePath.parent_path();
			filename = basePath.filename().string();
		}
	} else {
		candidates.push_back(fs::path(basePath.string() + ".gltf"));
		candidates.push_back(fs::path(basePath.string() + ".glb"));
	}

	for (const auto& candidate : candidates) {
		if (fs::exists(candidate)) {
			directoryPath = candidate.parent_path();
			filename = candidate.filename().string();
			break;
		}
	}

	std::unique_ptr<Model> model = std::make_unique<Model>();
	model->LoadObjFileAssimp(directoryPath.string(), filename);
	model->Initialize(modelCommon_.get());
	models.insert(std::make_pair(filePath, std::move(model)));


}

Model* ModelManager::FindModel(const std::string& filePath){
	if (models.contains(filePath)) {
		return models.at(filePath).get();
	}
	return nullptr;
}

void ModelManager::Finalize(){

	instance.reset();
	

}