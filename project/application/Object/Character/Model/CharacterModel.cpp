#include "CharacterModel.h"
#include "Model/ModelManager.h"
void CharacterModel::LoadModel() { 
	ModelManager::GetInstance()->LoadGltfModel("Resources/3d", "sizuku"); 
}