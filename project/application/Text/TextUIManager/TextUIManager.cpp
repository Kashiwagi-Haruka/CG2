#define NOMINMAX
#include "TextUIManager.h"
#include "DirectXCommon.h"
#include "GameObject/Door/Door.h"
#include "GameObject/Coffee/CoffeeTrivia.h"
#include "GameObject/Edamame/Edamame.h"
#include "GameObject/Edamame/EdamameTrivia.h"
#include "GameObject/Flashlight/Flashlight.h"
#include "GameObject/Key/Key.h"
#include "SpriteCommon.h"
#include <algorithm>

#include <codecvt>
#include <locale>

TextUIManager::TextUIManager() {

	fontHandle_ = FreeTypeManager::CreateFace("Resources/TD3_3102/Irohakaku/irohakakuC-Medium.ttf", 0);
	FreeTypeManager::SetPixelSizes(fontHandle_, 24, 24);
	text_.Initialize(fontHandle_);
	text_.SetString(U"p-再打刻");
	text_.SetPosition({640, 240});
	text_.SetColor({1, 1, 1, 1});
	text_.SetAlign(TextAlign::Center);
	text_.SetBlendMode(BlendMode::kBlendModeAlpha);
	text_.UpdateLayout();

	edamameTrivia_.Initialize(fontHandle_);
	edamameTrivia_.SetPosition({640, 512 + 64});
	edamameTrivia_.SetColor({1, 1, 1, 1});
	edamameTrivia_.SetAlign(TextAlign::Center);
	edamameTrivia_.SetBlendMode(BlendMode::kBlendModeAlpha);
	edamameTrivia_.UpdateLayout();

	coffeeTrivia_.Initialize(fontHandle_);
	coffeeTrivia_.SetPosition({640, 512 + 64});
	coffeeTrivia_.SetColor({1, 1, 1, 1});
	coffeeTrivia_.SetAlign(TextAlign::Center);
	coffeeTrivia_.SetBlendMode(BlendMode::kBlendModeAlpha);
	coffeeTrivia_.UpdateLayout();
}

TextUIManager::~TextUIManager() {}

void TextUIManager::Initialize() {
	isDraw_ = false;
	showTimer_ = showTime_;
	edamameTriviaAlpha_ = 0.0f;
	coffeeTriviaAlpha_ = 0.0f;
	wasEdamameRayHit_ = false;
	wasCoffeeRayHit_ = false;
}

void TextUIManager::Update() {
	if (isDraw_) {
		const float deltaTime = SpriteCommon::GetInstance()->GetDxCommon()->GetDeltaTime();
		showTimer_ -= deltaTime;
		if (showTimer_ <= 0.0f) {
			showTimer_ = showTime_;
			isDraw_ = false;
		}
	}

	if (Door::GetOpenMassage()) {
		text_.SetString(U"扉が開いた。");
		StartText();
	} else if (Key::GetGetKeyMessage()) {
		text_.SetString(U"鍵を入手した。");
		StartText();
	} else if (Door::GetLockMassage()) {
		text_.SetString(U"鍵がかかっている。");
		StartText();
	} else if (Flashlight::IsGetLightMessage()) {
		text_.SetString(U"懐中電灯を取得した。");
		StartText();
	}

	if (EdamameTrivia::GetIsSendStartTriviaMessage()) {
		edamameTrivia_.SetString(EdamameTrivia::GetString());
		edamameTrivia_.StartTyping(0.1f); // 0.1秒ごとに1文字ずつ表示
	}

	if (CoffeeTrivia::GetIsSendStartTriviaMessage()) {
		coffeeTrivia_.SetString(CoffeeTrivia::GetString());
		coffeeTrivia_.StartTyping(0.1f); // 0.1秒ごとに1文字ずつ表示
	}

	const float deltaTime = SpriteCommon::GetInstance()->GetDxCommon()->GetDeltaTime();
	const bool isEdamameRayHit = Edamame::IsRayHit();
	const bool isCoffeeRayHit = CoffeeTrivia::IsRayHit();
	if (isEdamameRayHit) {
		edamameTriviaAlpha_ = 1.0f;
	} else if ((wasEdamameRayHit_ || edamameTriviaAlpha_ > 0.0f) && EdamameTrivia::GetIsCurrentVoiceFinished()) {
		const float fadeStep = deltaTime / kEdamameFadeDuration_;
		edamameTriviaAlpha_ = std::max(0.0f, edamameTriviaAlpha_ - fadeStep);
	}
	wasEdamameRayHit_ = isEdamameRayHit;

	if (isCoffeeRayHit) {
		coffeeTriviaAlpha_ = 1.0f;
	} else if ((wasCoffeeRayHit_ || coffeeTriviaAlpha_ > 0.0f) && CoffeeTrivia::GetIsCurrentVoiceFinished()) {
		const float fadeStep = deltaTime / kEdamameFadeDuration_;
		coffeeTriviaAlpha_ = std::max(0.0f, coffeeTriviaAlpha_ - fadeStep);
	}
	wasCoffeeRayHit_ = isCoffeeRayHit;

	edamameTrivia_.SetColor({1.0f, 1.0f, 1.0f, edamameTriviaAlpha_});
	coffeeTrivia_.SetColor({1.0f, 1.0f, 1.0f, coffeeTriviaAlpha_});

	text_.Update(true);
	edamameTrivia_.Update();
	coffeeTrivia_.Update();
}

void TextUIManager::Draw() {
	if (isDraw_) {
		text_.Draw();
	}

	if (!ChairMenu::GetIsShowMenu()) {
		if (EdamameTrivia::GetIsDraw() && !EdamameTrivia::GetIsDie()) {
			// 死んでないとき且つ描画するとき
			edamameTrivia_.Draw();
		}
		if (CoffeeTrivia::IsRayHit() || coffeeTriviaAlpha_ > 0.0f) {
			coffeeTrivia_.Draw();
		}
	}
}

void TextUIManager::ShowKeyLostAtStageStartMessage() {
	text_.SetString(U"カギをなくした。");
	StartText(5.0f);
}

void TextUIManager::StartText(float showTime) {
	showTime_ = showTime;
	showTimer_ = showTime_;
	text_.StartTyping(0.05f); // 0.05秒ごとに1文字ずつ表示
	isDraw_ = true;
}