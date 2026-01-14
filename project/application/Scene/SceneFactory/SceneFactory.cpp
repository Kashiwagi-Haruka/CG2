#include "SceneFactory.h"
#include <memory>
#include "Scene/TitleScene.h"
#include "Scene/GameScene.h"
#include "Scene/ResultScene.h"
#include "Scene/GameOverScene.h"
#include "Scene/SampleScene/SampleScene.h"
BaseScene* SceneFactory::CreateScene(const std::string& sceneName) {
	std::shared_ptr<BaseScene> scene = nullptr;

	if (sceneName == "Title") {
		scene = std::make_shared<TitleScene>();
	}
	else if (sceneName == "Game") {
		scene = std::make_shared<GameScene>();
	}
	else if (sceneName == "Result") {
		scene = std::make_shared<ResultScene>();
	} else if (sceneName == "GameOver") {
		scene = std::make_shared<GameOverScene>();
	} else if (sceneName == "Sample") {
		scene = std::make_shared<SampleScene>();
	}


	return scene.get();
}