#include "ModelManeger.h"
#include "ModelCommon.h"
#include "Model.h"
std::unique_ptr<ModelManeger> ModelManeger::instance = nullptr;

ModelManeger* ModelManeger::GetInstance(){
	if (instance == nullptr) {
		instance = std::make_unique<ModelManeger>();
	}
	return instance.get();
}

void ModelManeger::Initialize(DirectXCommon* dxCommon){
	modelCommon_ = std::make_unique<ModelCommon>();
	modelCommon_->Initialize(dxCommon);
}

void ModelManeger::LoadModel(const std::string& filePath){

	if (models.contains(filePath)) {
		return;
	}

	std::unique_ptr<Model> model = std::make_unique<Model>();
	model->LoadObjFile("Resources/3d", filePath);
	model->Initialize(modelCommon_.get());
	models.insert(std::make_pair(filePath, std::move(model)));
}

Model* ModelManeger::FindModel(const std::string& filePath){
	if (models.contains(filePath)) {
		return models.at(filePath).get();
	}
	return nullptr;
}

void ModelManeger::Finalize(){

	instance.reset();
	

}