#include "Model/ModelManager.h"
#include "Model/ModelCommon.h"
#include "Model/Model.h"

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

void ModelManager::LoadModel(const std::string& filePath){

	if (models.contains(filePath)) {
		return;
	}

	std::unique_ptr<Model> model = std::make_unique<Model>();
	model->LoadObjFileAssimp("Resources/3d", filePath + ".obj");
	model->Initialize(modelCommon_.get());
	models.insert(std::make_pair(filePath, std::move(model)));
}
void ModelManager::LoadGltfModel(const std::string& filePath) {
	if (models.contains(filePath)) {
		return;
	}

	std::unique_ptr<Model> model = std::make_unique<Model>();
	model->LoadObjFileAssimp("Resources/3d", filePath +".glb");
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