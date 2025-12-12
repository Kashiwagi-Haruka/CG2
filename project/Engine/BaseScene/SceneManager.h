#pragma once
#include "BaseScene.h"
#include  "AbstractSceneFactory.h"
#include <memory>
class SceneManager {

	static std::unique_ptr<SceneManager> instance_;

	std::unique_ptr<BaseScene> scene_ = nullptr;
	std::unique_ptr<BaseScene> nextscene_ = nullptr;

	AbstractSceneFactory* sceneFactory_ = nullptr;

	public:

		void SetSceneFactory(AbstractSceneFactory* sceneFactory) { sceneFactory_ = sceneFactory; };
	    void ChangeScene(const std::string& sceneName);		
	    void Update();
	    void Draw();
	    void Finalize();
	    static SceneManager* GetInstance();
};
