#include "SceneTransition.h"

#include "GameBase.h"
#include "ScreenSize/ScreenSize.h"
#include "Sprite/SpriteCommon.h"
#include "TextureManager.h"

#include <algorithm>

SceneTransition::SceneTransition() {
	fadeTextureHandle_ = TextureManager::GetInstance()->GetTextureIndexByfilePath("Resources/TD3_3102/2d/white2x2.png");
	fadeSprite_ = std::make_unique<Sprite>();
	fadeSprite_->Initialize(fadeTextureHandle_);
	fadeSprite_->SetColor({0.0f, 0.0f, 0.0f, 1.0f});
}

SceneTransition::~SceneTransition() {}

void SceneTransition::Initialize(bool isIn) {
	isIn_ = isIn;
	isEnd = false;
	timer_ = 0.0f;
}

void SceneTransition::Update() {
	if (isEnd) {
		return;
	}

	timer_ += GameBase::GetInstance()->GetDeltaTime();
	if (timer_ >= duration_) {
		isEnd = true;
	}
}

void SceneTransition::Draw() {
	const float progress = GetProgress();
	float alpha = isIn_ ? progress : (1.0f - progress);
	alpha = std::clamp(alpha, 0.0f, 1.0f);
	if (alpha <= 0.0f) {
		return;
	}

	SpriteCommon::GetInstance()->DrawCommon();
	SpriteCommon::GetInstance()->SetBlendMode(BlendMode::kBlendModeAlpha);

	fadeSprite_->SetColor({0.0f, 0.0f, 0.0f, alpha});
	fadeSprite_->SetScale({SCREEN_SIZE::WIDTH, SCREEN_SIZE::HEIGHT});
	fadeSprite_->SetPosition({0.0f, 0.0f});
	fadeSprite_->Update();
	fadeSprite_->Draw();
}

float SceneTransition::GetProgress() const {
	if (duration_ <= 0.0f) {
		return 1.0f;
	}
	return std::clamp(timer_ / duration_, 0.0f, 1.0f);
}