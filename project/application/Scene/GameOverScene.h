#include "Audio.h"
#include "BaseScene.h"
#include "SceneTransition/SceneTransition.h"
#include "Sprite.h"
#include "Text/Text.h"
#include <imgui.h>
#include <memory>
class GameBase;
class GameOverScene : public BaseScene {
private:
	std::unique_ptr<SceneTransition> transition = nullptr;
	bool isTransitionIn = false;
	bool isTransitionOut = false;
	SoundData BGM_;
	bool isBGMPlaying = false;
	std::unique_ptr<Sprite> sprite_ = nullptr;
	uint32_t textureHandle_ = 0;
	uint32_t fontHandle_ = 0;
	Text gameOverText_{};

public:
	GameOverScene();
	~GameOverScene() override = default;
	void Initialize() override;
	void Update() override;
	void Draw() override;
	void Finalize() override;
};