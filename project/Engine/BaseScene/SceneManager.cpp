#include "SceneManager.h"
#include <cassert>
SceneManager* SceneManager::instance_ = nullptr;
SceneManager* SceneManager::GetInstance() {

	if (instance_ == nullptr) {
		instance_ = new SceneManager();
	}
	return instance_;
}

void SceneManager::Finalize(){

	
	scene_->Finalize();
	delete scene_;
	delete instance_;
	instance_ = nullptr;
}


void SceneManager::Update() { 
	
	if (nextscene_) {
	
		if (scene_) {
			scene_->Finalize();
			delete scene_;
		}

		scene_ = nextscene_;
		nextscene_ = nullptr;
		scene_->SetSceneManager(this);
		scene_->Initialize();
		
	}




	scene_->Update(); 


}

void SceneManager::Draw() { scene_->Draw(); }

void SceneManager::ChangeScene(const std::string& sceneName) { 

	assert(sceneFactory_);
	assert(nextscene_==nullptr);
	nextscene_=sceneFactory_->CreateScene(sceneName);	

}