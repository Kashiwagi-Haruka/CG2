#include "Menu.h"
#include "GameObject/KeyBindConfig.h"
#include "application/Color/Color.h"
#include "GameBase.h"
#include "WinApp.h"
#include "Sprite/SpriteCommon.h"
void Menu::ChangeMenu(std::string MenuName) {
	currentMenuName_ = MenuName; 
}
void Menu::Initialize() {
	// フォントハンドル
	fontHandle_ = FreeTypeManager::CreateFace("Resources/TD3_3102/Irohakaku/irohakakuC-Medium.ttf", 0);
	FreeTypeManager::SetPixelSizes(fontHandle_, 64, 64);
	currentMenuName_ = "Option"; 
	option_ = std::make_unique<Option>();
	option_->Initialize();

	menuText_.Initialize(fontHandle_);
	menuText_.SetString(U"p-再打刻");
	menuText_.SetPosition({WinApp::kClientWidth/2.0f,WinApp::kClientHeight/2.0f});
	menuText_.SetColor(COLOR::WHITE);
	menuText_.SetAlign(TextAlign::Center);


}

void Menu::Update() {
	menuText_.Update();
	if (currentMenuName_ == "Game") {
		// ゲームに戻る
	} else if (currentMenuName_ == "Save") {
		// セーブ
	} else if (currentMenuName_ == "Option") {
		// オプション
	} else if (currentMenuName_ == "Title") {
		// タイトル
	} else if (currentMenuName_ == "GameEnd") {
		// ゲームを終わる
	}
	if (isTrigger_) {
		if (currentMenuName_ == "Game") {
			// ゲームに戻る
		} else if (currentMenuName_ == "Save") {
			// セーブ
		} else if (currentMenuName_ == "Option") {
			// オプション
			option_->Update();
		} else if (currentMenuName_ == "Title") {
			// タイトル
		} else if (currentMenuName_ == "GameEnd") {
			// ゲームを終わる
		}
	}
}

void Menu::Draw() {
	SpriteCommon::GetInstance()->DrawCommonFont();
	menuText_.Draw();
	if (currentMenuName_ == "Game") {
		// ゲームに戻る
	} else if (currentMenuName_ == "Save") {
		// セーブ
	} else if (currentMenuName_ == "Option") {
		// オプション
	} else if (currentMenuName_ == "Title") {
		// タイトル
	} else if (currentMenuName_ == "GameEnd") {
		// ゲームを終わる
	}
	if (isTrigger_) {
		if (currentMenuName_ == "Game") {
			// ゲームに戻る
		} else if (currentMenuName_ == "Save") {
			// セーブ
		} else if (currentMenuName_ == "Option") {
			// オプション
			option_->Draw();
		} else if (currentMenuName_ == "Title") {
			// タイトル
		} else if (currentMenuName_ == "GameEnd") {
			// ゲームを終わる
		}
	
	}

}