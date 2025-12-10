#pragma once
#include "BaseScene.h"
#include  "AbstractSceneFactory.h"
class SceneManager {

	static SceneManager* instance_;

	BaseScene* scene_ = nullptr;
	BaseScene* nextscene_ = nullptr;

	AbstractSceneFactory* sceneFactory_ = nullptr;

	public:

		void SetSceneFactory(AbstractSceneFactory* sceneFactory) { sceneFactory_ = sceneFactory; };
	    void ChangeScene(const std::string& sceneName);		
	    void Update();
	    void Draw();
	    void Finalize();
	    static SceneManager* GetInstance();
};
