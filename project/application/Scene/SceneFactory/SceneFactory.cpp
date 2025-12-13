#include "SceneFactory.h"
#include "Scene/TitleScene.h"
#include "Scene/GameScene.h"
#include "Scene/ResultScene.h"
#include "Scene/GameOverScene.h"
#include "Scene/SampleScene/SampleScene.h"
BaseScene* SceneFactory::CreateScene(const std::string& sceneName) {
	BaseScene* scene = nullptr;

	if (sceneName == "Title") {
		scene = new TitleScene();
	}
	else if (sceneName == "Game") {
		scene = new GameScene();
	}
	else if (sceneName == "Result") {
		scene = new ResultScene();
	} else if (sceneName == "GameOver") {
		scene = new GameOverScene();
	} else if (sceneName == "Sample") {
		scene = new SampleScene();
	}


	return scene;
}