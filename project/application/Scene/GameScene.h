#pragma once
#include "Audio.h"
#include "BaseScene.h"
#include "GameBase.h"
#include "Light/DirectionalLight/DirectionalLight.h"
#include "Light/PointLight/PointLight.h"
#include "Light/SpotLight/SpotLight.h"
#include "Object3d/Object3d.h"
#include "Pause/Pause.h"
#include "SceneTransition/SceneTransition.h"
#include "Sprite.h"
#include "Vector2.h"
#include <imgui.h>
#include <string>

class GameScene : public BaseScene {

private:

	std::unique_ptr<SceneTransition> sceneTransition;

	std::unique_ptr<Pause> pause;



	bool isTransitionIn = true;
	bool isTransitionOut = false;
	std::string nextSceneName;
	bool isBGMPlaying = false;
	bool isPause = false;

	SoundData BGMData;


	DirectionalLight directionalLight_{};
	PointLight pointLights_{};
	SpotLight spotLights_{};

	bool sceneEndClear = false;
	bool sceneEndOver = false;

public:
	GameScene();
	~GameScene() override;

	void Initialize() override;
	void Update() override;
	void Draw() override;
	void Finalize() override;

	void DebugImGui();
};
