#pragma once
#include "AbstractSceneFactory.h"
#include "BaseScene.h"
#include "Sprite.h"
#include <memory>
#include <string>
#include <vector>
class SceneManager {

	static std::unique_ptr<SceneManager> instance_;

	std::unique_ptr<BaseScene> scene_ = nullptr;
	std::unique_ptr<BaseScene> nextscene_ = nullptr;
	std::string currentSceneName_;
	std::string nextSceneName_;

	AbstractSceneFactory* sceneFactory_ = nullptr;
	bool isSceneReinitializeRequested_ = false;

	enum class TransitionState { None, FadingOut, FadingIn };
	TransitionState transitionState_ = TransitionState::None;
	float transitionTimer_ = 0.0f;
	const float fadeOutDuration_ = 0.35f;
	const float fadeInDuration_ = 0.35f;

	std::unique_ptr<Sprite> fadeSprite_ = nullptr;
	uint32_t fadeTextureHandle_ = 0;

	void EnsureFadeSprite();
	float GetFadeAlpha() const;

public:
	void SetSceneFactory(AbstractSceneFactory* sceneFactory) { sceneFactory_ = sceneFactory; };
	void ChangeScene(const std::string& sceneName);
	void RequestReinitializeCurrentScene();
	void Update();
	void Draw();
	void DrawOverlay();
	void Finalize();
	const std::string& GetCurrentSceneName() const { return currentSceneName_; }
	std::vector<std::string> GetSceneNames() const;
	static SceneManager* GetInstance();
};