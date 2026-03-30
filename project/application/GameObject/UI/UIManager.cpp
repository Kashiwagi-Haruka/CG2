#include "UIManager.h"
#include "GameObject/Flashlight/Flashlight.h"
#include "GameObject/Key/Key.h"
#include "Input.h"
#include "SceneManager.h"
#include "application/GameObject/SEManager/SEManager.h"

bool UIManager::isPause_ = false;

void UIManager::TogglePause() {
	// Tabキーでポーズ
	isPause_ = (isPause_) ? false : true;
}

UIManager::UIManager() {
	// UIManager
	textUIManager_ = std::make_unique<TextUIManager>();
	menu_ = std::make_unique<Menu>();
	raySprite_ = std::make_unique<RaySprite>();
	tabKey_ = std::make_unique<TabKey>();

	keyIcon_ = std::make_unique<KeyIcon>();
	lightIcon_ = std::make_unique<LightIcon>();
	mission_ = std::make_unique<Mission>();
}

void UIManager::Initialize() {
	isPause_ = false;
	// UIManager
	textUIManager_->Initialize();
	menu_->Initialize();
	raySprite_->Initialize();
	tabKey_->Initialize();
	keyIcon_->Initialize();
	lightIcon_->Initialize();
	mission_->Initialize();
}

void UIManager::Update() {
	// カーソルを画面中央に設定する
	auto* input = Input::GetInstance();

	if (input->TriggerKey(DIK_TAB)) {

		TogglePause();

		if (isPause_) {
			CursorShowAndMove();
		} else {
			CursorHideAndStop();
		}
		SEManager::SoundPlay(SEManager::PUSH_WATCH);
	}

	if (UIManager::GetIsPause()) {
		menu_->Update();
		const Menu::Action menuAction = menu_->ConsumePendingAction();
		if (menuAction == Menu::Action::kResumeGame) {
			isPause_ = false;
			CursorHideAndStop();
		} else if (menuAction == Menu::Action::kBackToTitle) {
			isPause_ = false;
			CursorHideAndStop();
			SceneManager::GetInstance()->ChangeScene("Title");
		} else if (menuAction == Menu::Action::kEndGame) {
			PostQuitMessage(0);
		}
	}

	// Text
	textUIManager_->Update();
	raySprite_->Update();
	tabKey_->Update();
	keyIcon_->Update();
	lightIcon_->Update();
	mission_->Update();
}

void UIManager::CloseOptionANdPrepareResume() {
	if (isPause_ && menu_ && menu_->IsOptionOpen()) {
		menu_->CloseOptionAndPrepareResume();
	}
}

void UIManager::CursorShowAndMove() {
	// カーソルを画面中央に設定する
	auto* input = Input::GetInstance();
	input->SetIsCursorVisible(true);
	input->SetIsCursorStability(false);
}

void UIManager::CursorHideAndStop() { // カーソルを画面中央に設定する
	auto* input = Input::GetInstance();
	input->SetIsCursorVisible(false);
	input->SetIsCursorStability(true);
}

void UIManager::Draw() {
	textUIManager_->Draw();

	raySprite_->Draw();

	if (isPause_) {
		menu_->Draw();
	} else {
		tabKey_->Draw();
	}

	if (Flashlight::IsGetLight()) {
		lightIcon_->Draw();
	}

	if (Key::IsGetKey()) {
		keyIcon_->Draw();
	}

	mission_->Draw();
}