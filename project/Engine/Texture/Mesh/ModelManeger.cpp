#include "ModelManeger.h"
#include "ModelCommon.h"
#include "Model.h"
ModelManeger* ModelManeger::instance = nullptr;

ModelManeger* ModelManeger::GetInstance(){
	if (instance == nullptr) {
		instance = new ModelManeger;
	}
	return instance;
}

void ModelManeger::Initialize(DirectXCommon* dxCommon){
	modelCommon_ = new ModelCommon();
	modelCommon_->Initialize(dxCommon);
}

void ModelManeger::LoadModel(const std::string& filePath){

	if (models.contains(filePath)) {
		return;
	}

	std::unique_ptr<Model> model = std::make_unique<Model>();
	model->LoadObjFile("Resources/3d", filePath);
	model->Initialize(modelCommon_);
	models.insert(std::make_pair(filePath, std::move(model)));
}

Model* ModelManeger::FindModel(const std::string& filePath){
	if (models.contains(filePath)) {
		return models.at(filePath).get();
	}
	return nullptr;
}

void ModelManeger::Finalize(){

	delete instance;
	instance = nullptr;

}