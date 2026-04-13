#include "SceneManager.h"
#include "Color/Color.h"
#include "Engine/Editor/EditorTool/Hierarchy/Hierarchy.h"
#include "GameBase.h"
#include "ScreenSize/ScreenSize.h"
#include "Sprite/SpriteCommon.h"
#include "TextureManager.h"
#include <algorithm>
#include <cassert>

std::unique_ptr<SceneManager> SceneManager::instance_ = nullptr;

SceneManager* SceneManager::GetInstance() {
	if (!instance_) {
		instance_ = std::make_unique<SceneManager>();
	}
	return instance_.get();
}

void SceneManager::RequestReinitializeCurrentScene() { isSceneReinitializeRequested_ = true; }

void SceneManager::Finalize() {

	if (scene_) {
		scene_->Finalize();
		scene_.reset();
	}

	nextscene_.reset();
	currentSceneName_.clear();
	nextSceneName_.clear();

	// ★ Singleton の instance を解放
	instance_.reset();
}

void SceneManager::Update() {
	const float deltaTime = GameBase::GetInstance()->GetDeltaTime();

	// シーン再初期化
	if (isSceneReinitializeRequested_ && scene_) {
		scene_->Finalize();
		scene_->Initialize();
		isSceneReinitializeRequested_ = false;
	}

	// シーン切り替え（全シーン共通フェード）
	if (nextscene_ && scene_ && transitionState_ == TransitionState::None) {
		transitionState_ = TransitionState::FadingOut;
		transitionTimer_ = 0.0f;
	}

	if (nextscene_ && !scene_) {
		scene_ = std::move(nextscene_);
		currentSceneName_ = nextSceneName_;
		nextSceneName_.clear();
		scene_->SetSceneManager(this);
		scene_->Initialize();
		transitionState_ = TransitionState::FadingIn;
		transitionTimer_ = 0.0f;
		isSceneReinitializeRequested_ = false;
	}

	if (transitionState_ == TransitionState::FadingOut) {
		transitionTimer_ += deltaTime;
		if (transitionTimer_ >= fadeOutDuration_) {
			if (scene_) {
				scene_->Finalize();
			}

			scene_ = std::move(nextscene_);
			currentSceneName_ = nextSceneName_;
			nextSceneName_.clear();
			scene_->SetSceneManager(this);
			scene_->Initialize();

			transitionState_ = TransitionState::FadingIn;
			transitionTimer_ = 0.0f;
			isSceneReinitializeRequested_ = false;
		}
	} else if (transitionState_ == TransitionState::FadingIn) {
		transitionTimer_ += deltaTime;
		if (transitionTimer_ >= fadeInDuration_) {
			transitionState_ = TransitionState::None;
			transitionTimer_ = 0.0f;
		}
	}

	if (scene_) {
		Hierarchy* hierarchy = Hierarchy::GetInstance();
		if (hierarchy && hierarchy->IsEditorPreviewActive()) {
			hierarchy->UpdateEditorPreview();
		} else {
			scene_->Update();
		}
	}
}

void SceneManager::Draw() {
	if (scene_) {
		scene_->Draw();
	}
}

void SceneManager::DrawOverlay() {
	const float alpha = GetFadeAlpha();
	if (alpha <= 0.0f) {
		return;
	}

	EnsureFadeSprite();
	SpriteCommon::GetInstance()->DrawCommon();
	SpriteCommon::GetInstance()->SetBlendMode(BlendMode::kBlendModeAlpha);
	fadeSprite_->SetColor({0.0f, 0.0f, 0.0f, alpha});
	fadeSprite_->SetScale({SCREEN_SIZE::WIDTH, SCREEN_SIZE::HEIGHT});
	fadeSprite_->SetPosition({0.0f, 0.0f});
	fadeSprite_->Update();
	fadeSprite_->Draw();
}

void SceneManager::ChangeScene(const std::string& sceneName) {

	assert(sceneFactory_);
	assert(nextscene_ == nullptr);

	nextscene_ = sceneFactory_->CreateScene(sceneName);
	nextSceneName_ = sceneName;
}

void SceneManager::EnsureFadeSprite() {
	if (fadeSprite_) {
		return;
	}

	fadeTextureHandle_ = TextureManager::GetInstance()->GetTextureIndexByfilePath("Resources/TD3_3102/2d/white2x2.png");
	fadeSprite_ = std::make_unique<Sprite>();
	fadeSprite_->Initialize(fadeTextureHandle_);
	fadeSprite_->SetColor({0,0,0,1});
}

float SceneManager::GetFadeAlpha() const {
	if (transitionState_ == TransitionState::FadingOut) {
		return std::clamp(transitionTimer_ / fadeOutDuration_, 0.0f, 1.0f);
	}
	if (transitionState_ == TransitionState::FadingIn) {
		return std::clamp(1.0f - (transitionTimer_ / fadeInDuration_), 0.0f, 1.0f);
	}
	return 0.0f;
}
std::vector<std::string> SceneManager::GetSceneNames() const {
	if (!sceneFactory_) {
		return {};
	}
	return sceneFactory_->GetSceneNames();
}