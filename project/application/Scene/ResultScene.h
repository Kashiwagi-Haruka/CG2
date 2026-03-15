#pragma once
#include "Audio.h"
#include "BaseScene.h"
#include "SceneTransition/SceneTransition.h"
#include "Sprite.h"
#include <imgui.h>
#include <memory>
class GameBase;

class ResultScene : public BaseScene {
	std::unique_ptr<SceneTransition> transition = nullptr;
	bool isTransitionIn = false;
	bool isTransitionOut = false;

	SoundData BGM_;
	bool isBGMPlaying = false;

	std::unique_ptr<Sprite> sprite_ = nullptr;
	uint32_t textureHandle_ = 0;
public:
	ResultScene();
	~ResultScene() override = default;
	void Initialize() override;
	void Update() override;
	void Draw() override;
	void Finalize() override;
};