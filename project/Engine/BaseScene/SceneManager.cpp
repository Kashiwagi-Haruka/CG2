#include "SceneManager.h"
#include <cassert>

std::unique_ptr<SceneManager> SceneManager::instance_ = nullptr;

SceneManager* SceneManager::GetInstance() {
	if (!instance_) {
		instance_ = std::make_unique<SceneManager>();
	}
	return instance_.get();
}

void SceneManager::Finalize() {

	if (scene_) {
		scene_->Finalize();
		scene_.reset();
	}

	nextscene_.reset();

	// ★ Singleton の instance を解放
	instance_.reset();
}

void SceneManager::Update() {

	// シーン切り替え
	if (nextscene_) {

		if (scene_) {
			scene_->Finalize();
		}

		scene_ = std::move(nextscene_);
		scene_->SetSceneManager(this);
		scene_->Initialize();
	}

	if (scene_) {
		scene_->Update();
	}
}

void SceneManager::Draw() {
	if (scene_) {
		scene_->Draw();
	}
}

void SceneManager::ChangeScene(const std::string& sceneName) {

	assert(sceneFactory_);
	assert(nextscene_ == nullptr);

	// ★ unique_ptr を直接受け取る
	nextscene_ = std::unique_ptr<BaseScene>(sceneFactory_->CreateScene(sceneName));
}
