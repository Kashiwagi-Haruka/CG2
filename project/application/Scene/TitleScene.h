#pragma once
#include "Audio.h"
#include "BaseScene.h"
#include "Camera.h"
#include "GameObject/TimeCard/TimeCard.h"
#include "GameObject/TimeCard/TimeCardRack.h"
#include "GameObject/UI/GameContinued/GameContinued.h"
#include "GameObject/Wall/Wall.h"
#include "GameObject/YoshidaMath/RandomClass.h"
#include "Light/CommonLight/DirectionalCommonLight.h"
#include "SceneTransition/SceneTransition.h"
#include "Sprite.h"
#include "Text/FirstStory/FirstStory.h"
#include "Text/TItleMenu/TitleMenuUI.h"
#include <memory>
#include"Text/TitleTimeText/TitleTimeText.h"

class GameBase;

class TitleScene : public BaseScene {
private:
	bool isTransitionIn = false;
	bool isTransitionOut = false;
	std::unique_ptr<SceneTransition> transition = nullptr;
	std::unique_ptr<TitleMenuUI> titleMenuUI_ = nullptr;
	std::unique_ptr<FirstStory> firstStory_ = nullptr;
	std::unique_ptr<TitleTimeText> titleTimeText_ = nullptr;

	std::unique_ptr<TimeCard> timeCard_ = nullptr;
	std::unique_ptr<TimeCardRack> timeCardRack_ = nullptr;
	std::unique_ptr<Wall> wall_ = nullptr;
	std::unique_ptr<GameContinued> gameContinued_ = nullptr;
	bool isGameContinuedOpen_ = false;

	Matrix4x4 identityMat_;
	// DirectionalLight
	DirectionalCommonLight directionalLight_{};
	// カメラ
	std::unique_ptr<Camera> camera_ = nullptr;
	Transform cameraTransform_;
	Vector3 cameraDefaultPos_ = {0.0f};
	Vector3 cameraRandomOffset_ = {0.0f};
	float cameraMoveTimer_ = 0.0f;
	std::unique_ptr<RandomClass> random_ = nullptr;

private:
	void CameraUpdate();
	void TransitionStart();
public:
	TitleScene();
	~TitleScene() override = default;
	void Initialize() override;
	void Update() override;
	void Draw() override;
	void Finalize() override;
};