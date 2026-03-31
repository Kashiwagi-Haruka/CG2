#define NOMINMAX
#include "DamageOverlay.h"
#include "Sprite/SpriteCommon.h"
#include "TextureManager.h"
#include "WinApp.h"
#include <algorithm>

DamageOverlay::DamageOverlay() { sprite_ = std::make_unique<Sprite>(); }

void DamageOverlay::Initialize() {
	const uint32_t texture = TextureManager::GetInstance()->GetTextureIndexByfilePath("Resources/TD3_3102/2d/Damage.png");
	sprite_->Initialize(texture);
	sprite_->SetPosition({0.0f, 0.0f});
	sprite_->SetScale({static_cast<float>(WinApp::kClientWidth), static_cast<float>(WinApp::kClientHeight)});
	sprite_->SetColor({0.0f, 0.0f, 0.0f, 0.0f});
	sprite_->Update();
	displayTimer_ = 0.0f;
	forceDisplay_ = false;
	isInitialized_ = true;
}

void DamageOverlay::Update(float deltaTime, float hp, float maxHp) {
	if (!isInitialized_) {
		return;
	}
	displayTimer_ = std::max(0.0f, displayTimer_ - deltaTime);
	forceDisplay_ = (hp <= 2.0f);
	if (displayTimer_ <= 0.0f && !forceDisplay_) {
		return;
	}
	UpdateSpriteColor(hp, maxHp);
	sprite_->Update();
}

void DamageOverlay::Draw() const {
	if (!isInitialized_ || (displayTimer_ <= 0.0f && !forceDisplay_)) {
		return;
	}
	SpriteCommon::GetInstance()->DrawCommon();
	sprite_->Draw();
}

void DamageOverlay::StartDisplay() {
	if (!isInitialized_) {
		return;
	}
	displayTimer_ = kDisplayDuration_;
}

void DamageOverlay::UpdateSpriteColor(float hp, float maxHp) {
	const float clampedHp = std::clamp(hp, 0.0f, maxHp);
	Vector4 color = {0.0f, 0.0f, 0.0f, 0.0f};

	if (clampedHp >= 2.0f) {
		const float t = std::clamp(maxHp - clampedHp, 0.0f, 1.0f);
		color = {0.0f, 0.0f, 0.0f, t};
	} else {
		const float t = std::clamp((2.0f - clampedHp) / 2.0f, 0.0f, 1.0f);
		color = {t, 0.0f, 0.0f, 1.0f};
	}

	sprite_->SetColor(color);
}