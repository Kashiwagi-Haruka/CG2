#pragma once
#include "AbstractSceneFactory.h"
#include "BaseScene.h"
#include <memory>
class SceneManager {

	static std::unique_ptr<SceneManager> instance_;

	std::unique_ptr<BaseScene> scene_ = nullptr;
	std::unique_ptr<BaseScene> nextscene_ = nullptr;

	AbstractSceneFactory* sceneFactory_ = nullptr;
	bool isSceneReinitializeRequested_ = false;

public:
	void SetSceneFactory(AbstractSceneFactory* sceneFactory) { sceneFactory_ = sceneFactory; };
	void ChangeScene(const std::string& sceneName);
	void RequestReinitializeCurrentScene();
	void Update();
	void Draw();
	void Finalize();
	static SceneManager* GetInstance();
};