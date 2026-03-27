#include "Menu.h"
#include "GameBase.h"
#include "GameObject/KeyBindConfig.h"
#include "Sprite/SpriteCommon.h"
#include "WinApp.h"
#include "application/Color/Color.h"

namespace {
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
}

void Menu::Update() {
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

	if (isTrigger_ && currentMenuName_ == "Option") {
		option_->Update();
	}
}

void Menu::Draw() {
	SpriteCommon::GetInstance()->DrawCommonFont();
	menuText_.Draw();
	GameText_.Draw();
	SaveText_.Draw();
	OptionText_.Draw();
	TitleText_.Draw();
	GameEndText_.Draw();

	if (isTrigger_ && currentMenuName_ == "Option") {
		option_->Draw();
	}
}