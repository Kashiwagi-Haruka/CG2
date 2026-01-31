#pragma once
#include "BaseScene.h"
#include <array>
#include <cstdint>
#include <memory>

class CameraController;
class MapchipField;
class Player;
class SkyDome;
class Sprite;
class TutorialScene : public BaseScene {

public:
	TutorialScene();
	~TutorialScene() override = default;
	void Initialize() override;
	void Update() override;
	void Draw() override;
	void Finalize() override;

private:
	static constexpr int kStepCount = 6;
	static constexpr float kSkipHoldDuration = 1.0f;

	std::unique_ptr<CameraController> cameraController_;
	std::unique_ptr<SkyDome> skyDome_;
	std::unique_ptr<Player> player_;
	std::unique_ptr<MapchipField> field_;
	std::unique_ptr<Sprite> controlSprite_;
	uint32_t controlSpriteHandle_ = 0;

	std::array<bool, kStepCount> stepCompleted_{};
	int currentStepIndex_ = 0;
	bool isTutorialComplete_ = false;
	bool isPaused_ = false;
	bool wasSkipKeyHeld_ = false;
	float skipHoldTimer_ = 0.0f;
};