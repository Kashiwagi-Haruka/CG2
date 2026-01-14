#pragma once
#include "AbstractSceneFactory.h"
class SceneFactory : public AbstractSceneFactory {

	std::unique_ptr<BaseScene> CreateScene(const std::string& sceneName) override;

public:
	SceneFactory() = default;
	~SceneFactory() = default;
};