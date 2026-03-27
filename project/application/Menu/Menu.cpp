#include "Menu.h"
#include "GameBase.h"
#include "GameObject/KeyBindConfig.h"
#include "Sprite/SpriteCommon.h"
#include "WinApp.h"
#include "application/Color/Color.h"
#include "Input.h"
#include "TextureManager.h"

#include <array>

namespace {
constexpr std::array<const char*, 5> kMenuOrder = {"Game", "Save", "Option", "Title", "GameEnd"};

size_t FindMenuIndex(const std::string& menuName) {
	for (size_t i = 0; i < kMenuOrder.size(); ++i) {
		if (menuName == kMenuOrder[i]) {
			return i;
		}
	}
	return 0;
}

std::u32string ToMenuLabel(const std::string& menuName) {
	if (menuName == "Game") {
		return U"ゲームに戻る";
	}
	if (menuName == "Save") {
		return U"セーブ";
	}
	if (menuName == "Option") {
		return U"オプション";
	}
	if (menuName == "Title") {
		return U"タイトルへ戻る";
	}
	if (menuName == "GameEnd") {
		return U"ゲームを終了";
	}
	return U"";
}
} // namespace

void Menu::ChangeMenu(std::string MenuName) { currentMenuName_ = MenuName; }

void Menu::Initialize() {
	// フォントハンドル
	fontHandle_ = FreeTypeManager::CreateFace("Resources/TD3_3102/Irohakaku/irohakakuC-Medium.ttf", 0);
	FreeTypeManager::SetPixelSizes(fontHandle_, 48, 48);
	currentMenuName_ = "Option";
	option_ = std::make_unique<Option>();
	option_->Initialize();

	menuText_.Initialize(fontHandle_);
	menuText_.SetString(U"メニュー");
	menuText_.SetPosition({WinApp::kClientWidth / 2.0f, WinApp::kClientHeight / 2.0f - 180.0f});
	menuText_.SetColor(COLOR::WHITE);
	menuText_.SetAlign(TextAlign::Center);
	menuText_.UpdateLayout(false);

	GameText_.Initialize(fontHandle_);
	GameText_.SetString(ToMenuLabel("Game"));
	GameText_.SetPosition({WinApp::kClientWidth / 2.0f, WinApp::kClientHeight / 2.0f - 60.0f});
	GameText_.SetAlign(TextAlign::Center);
	GameText_.UpdateLayout(false);

	SaveText_.Initialize(fontHandle_);
	SaveText_.SetString(ToMenuLabel("Save"));
	SaveText_.SetPosition({WinApp::kClientWidth / 2.0f, WinApp::kClientHeight / 2.0f});
	SaveText_.SetAlign(TextAlign::Center);
	SaveText_.UpdateLayout(false);

	OptionText_.Initialize(fontHandle_);
	OptionText_.SetString(ToMenuLabel("Option"));
	OptionText_.SetPosition({WinApp::kClientWidth / 2.0f, WinApp::kClientHeight / 2.0f + 60.0f});
	OptionText_.SetAlign(TextAlign::Center);
	OptionText_.UpdateLayout(false);

	TitleText_.Initialize(fontHandle_);
	TitleText_.SetString(ToMenuLabel("Title"));
	TitleText_.SetPosition({WinApp::kClientWidth / 2.0f, WinApp::kClientHeight / 2.0f + 120.0f});
	TitleText_.SetAlign(TextAlign::Center);
	TitleText_.UpdateLayout(false);

	GameEndText_.Initialize(fontHandle_);
	GameEndText_.SetString(ToMenuLabel("GameEnd"));
	GameEndText_.SetPosition({WinApp::kClientWidth / 2.0f, WinApp::kClientHeight / 2.0f + 180.0f});
	GameEndText_.SetAlign(TextAlign::Center);
	GameEndText_.UpdateLayout(false);

	const uint32_t overlayTexture = TextureManager::GetInstance()->GetTextureIndexByfilePath("Resources/TD3_3102/2d/white2x2.png");
	backgroundOverlaySprite_ = std::make_unique<Sprite>();
	backgroundOverlaySprite_->Initialize(overlayTexture);
	backgroundOverlaySprite_->SetPosition({0.0f, 0.0f});
	backgroundOverlaySprite_->SetScale({static_cast<float>(WinApp::kClientWidth), static_cast<float>(WinApp::kClientHeight)});
	backgroundOverlaySprite_->SetColor({0.0f, 0.0f, 0.0f, 0.5f});
	backgroundOverlaySprite_->Update();
}

void Menu::Update() {
	backgroundOverlaySprite_->Update();
	auto* input = Input::GetInstance();
	if (isTrigger_ && currentMenuName_ == "Option") {
		option_->Update();
		if (!option_->GetIsShowOption()) {
			isTrigger_ = false;
		}
		return;
	}

	const bool moveUp = input->TriggerKey(DIK_W) || input->TriggerKey(DIK_UP);
	const bool moveDown = input->TriggerKey(DIK_S) || input->TriggerKey(DIK_DOWN);
	const float mouseWheelDelta = input->GetMouseWheelDelta();
	const bool wheelUp = mouseWheelDelta > 0.0f;
	const bool wheelDown = mouseWheelDelta < 0.0f;

	const int moveDirection = (moveDown || wheelDown) ? 1 : (moveUp || wheelUp) ? -1 : 0;
	if (moveDirection != 0) {
		const int menuCount = static_cast<int>(kMenuOrder.size());
		const int currentIndex = static_cast<int>(FindMenuIndex(currentMenuName_));
		int nextIndex = currentIndex + moveDirection;
		if (nextIndex < 0) {
			nextIndex += menuCount;
		} else if (nextIndex >= menuCount) {
			nextIndex -= menuCount;
		}
		currentMenuName_ = kMenuOrder[static_cast<size_t>(nextIndex)];
	}
	if (input->TriggerKey(DIK_E)) {
		if (currentMenuName_ == "Game") {
			pendingAction_ = Action::kResumeGame;
			isTrigger_ = false;
		} else if (currentMenuName_ == "Save") {
			pendingAction_ = Action::kSave;
			isTrigger_ = false;
		} else if (currentMenuName_ == "Option") {
			pendingAction_ = Action::kOpenOption;
			isTrigger_ = true;
			option_->OpenOption();
		} else if (currentMenuName_ == "Title") {
			pendingAction_ = Action::kBackToTitle;
			isTrigger_ = false;
		} else if (currentMenuName_ == "GameEnd") {
			pendingAction_ = Action::kEndGame;
			isTrigger_ = false;
		}
	}
	auto setDefaultColor = [](Text& text) {
		text.SetColor(COLOR::WHITE);
		text.UpdateLayout(false);
	};
	setDefaultColor(GameText_);
	setDefaultColor(SaveText_);
	setDefaultColor(OptionText_);
	setDefaultColor(TitleText_);
	setDefaultColor(GameEndText_);

	if (currentMenuName_ == "Game") {
		GameText_.SetColor(COLOR::RED);
		GameText_.UpdateLayout(false);
	} else if (currentMenuName_ == "Save") {
		SaveText_.SetColor(COLOR::RED);
		SaveText_.UpdateLayout(false);
	} else if (currentMenuName_ == "Option") {
		OptionText_.SetColor(COLOR::RED);
		OptionText_.UpdateLayout(false);
	} else if (currentMenuName_ == "Title") {
		TitleText_.SetColor(COLOR::RED);
		TitleText_.UpdateLayout(false);
	} else if (currentMenuName_ == "GameEnd") {
		GameEndText_.SetColor(COLOR::RED);
		GameEndText_.UpdateLayout(false);
	}
}
Menu::Action Menu::ConsumePendingAction() {
	const Action action = pendingAction_;
	pendingAction_ = Action::kNone;
	return action;
}
bool Menu::IsOptionOpen() const { return isTrigger_ && currentMenuName_ == "Option" && option_ && option_->GetIsShowOption(); }

void Menu::CloseOptionAndPrepareResume() {
	if (!IsOptionOpen()) {
		return;
	}
	option_->SaveOptionData();
	option_->CloseOption();
	isTrigger_ = false;
	currentMenuName_ = "Game";
	pendingAction_ = Action::kResumeGame;
}

void Menu::Draw() {
	SpriteCommon::GetInstance()->DrawCommon();
	backgroundOverlaySprite_->Draw();
	SpriteCommon::GetInstance()->DrawCommonFont();
	if (isTrigger_ && currentMenuName_ == "Option") {
		option_->Draw();
		return;
	}
	menuText_.Draw();
	GameText_.Draw();
	SaveText_.Draw();
	OptionText_.Draw();
	TitleText_.Draw();
	GameEndText_.Draw();
}