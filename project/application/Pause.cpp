#include "Pause.h"
#include "Function.h"
#include "GameBase.h"
#include "Input.h"
#include "TextureManager.h"

namespace {
constexpr float kTransitionSpeed = 0.06f;
constexpr float kHiddenOffsetX = 1280.0f;
} // namespace

Pause::Pause() {
	BGHandle_ = TextureManager::GetInstance()->GetTextureIndexByfilePath("Resources/2d/PauseBG.png");
	SelectHandle_ = TextureManager::GetInstance()->GetTextureIndexByfilePath("Resources/2d/PauseSelect.png");
	ButtonHandle_ = TextureManager::GetInstance()->GetTextureIndexByfilePath("Resources/2d/PauseButton.png");

	BG_ = std::make_unique<Sprite>();
	Select_ = std::make_unique<Sprite>();
	Button_ = std::make_unique<Sprite>();

	auto spriteCommon = GameBase::GetInstance()->GetSpriteCommon();
	BG_->Initialize(spriteCommon, BGHandle_);
	Select_->Initialize(spriteCommon, SelectHandle_);
	Button_->Initialize(spriteCommon, ButtonHandle_);
}

void Pause::Initialize() {

	startTime = 0.0f;
	isStart_ = false;
	isEnd_ = false;
	isActive_ = false;
	action_ = Action::kNone;
	selectIndex_ = 1;

	selectBasePos_ = {(1700.0f - selectSize_.x) / 2.0f, (900.0f - selectSize_.y) / 2.0f};
	buttonBasePos_[0] = {selectBasePos_.x - 60.0f, selectBasePos_.y + 60.0f};
	buttonBasePos_[1] = {selectBasePos_.x - 60.0f, selectBasePos_.y + 260.0f};

	BG_->SetScale(bgSize_);
	Select_->SetScale(selectSize_);
	Button_->SetScale(buttonSize_);
}

Pause::Action Pause::ConsumeAction() {
	Action action = action_;
	action_ = Action::kNone;
	return action;
}

void Pause::Update(bool isPause) {

	action_ = Action::kNone;

	if (isPause && !isActive_ && !isStart_) {
		startTime = 0.0f;
		isStart_ = true;
		isEnd_ = false;
	}

	if (!isPause && isActive_ && !isEnd_) {
		startTime = 0.0f;
		isEnd_ = true;
		isStart_ = false;
	}

	float offsetX = kHiddenOffsetX;
	if (isStart_) {
		startTime += kTransitionSpeed;
		offsetX = Function::Lerp(kHiddenOffsetX, 0.0f, startTime);
		if (startTime >= 1.0f) {
			startTime = 1.0f;
			isStart_ = false;
			isActive_ = true;
			offsetX = 0.0f;
		}
	} else if (isEnd_) {
		startTime += kTransitionSpeed;
		offsetX = Function::Lerp(0.0f, kHiddenOffsetX, startTime);
		if (startTime >= 1.0f) {
			startTime = 1.0f;
			isEnd_ = false;
			isActive_ = false;
			offsetX = kHiddenOffsetX;
		}
	} else if (isActive_) {
		offsetX = 0.0f;
	}

	if (isActive_) {
		bool moveUp = GameBase::GetInstance()->TriggerKey(DIK_W) || GameBase::GetInstance()->TriggerKey(DIK_UP) || GameBase::GetInstance()->TriggerButton(Input::PadButton::kButtonUp);
		bool moveDown = GameBase::GetInstance()->TriggerKey(DIK_S) || GameBase::GetInstance()->TriggerKey(DIK_DOWN) || GameBase::GetInstance()->TriggerButton(Input::PadButton::kButtonDown);

		if (moveUp || moveDown) {
			selectIndex_ = 1 - selectIndex_;
		}

		bool confirm = GameBase::GetInstance()->TriggerKey(DIK_RETURN) || GameBase::GetInstance()->TriggerKey(DIK_SPACE) || GameBase::GetInstance()->TriggerButton(Input::PadButton::kButtonA);
		bool cancel = GameBase::GetInstance()->TriggerKey(DIK_ESCAPE) || GameBase::GetInstance()->TriggerButton(Input::PadButton::kButtonStart) ||
		              GameBase::GetInstance()->TriggerButton(Input::PadButton::kButtonB);

		if (confirm) {
			action_ = (selectIndex_ == 0) ? Action::kTitle : Action::kResume;
		} else if (cancel) {
			action_ = Action::kResume;
		}
	}

	Vector2 bgPos = {bgBasePos_.x + offsetX, bgBasePos_.y};
	Vector2 selectPos = {selectBasePos_.x + offsetX, selectBasePos_.y};
	Vector2 buttonPos = {buttonBasePos_[selectIndex_].x + offsetX, buttonBasePos_[selectIndex_].y};

	BG_->SetPosition(bgPos);
	Select_->SetPosition(selectPos);
	Button_->SetPosition(buttonPos);

	BG_->Update();
	Select_->Update();
	Button_->Update();
}

void Pause::Draw() {

	if (!IsVisible()) {
		return;
	}

	BG_->Draw();
	Select_->Draw();
	Button_->Draw();
}