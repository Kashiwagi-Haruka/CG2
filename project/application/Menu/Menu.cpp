#include "Menu.h"
#include "GameObject/KeyBindConfig.h"
void Menu::ChangeMenu(std::string MenuName) {
	currentMenuName_ = MenuName; 
}
void Menu::Initialize() {
	currentMenuName_ = "Game"; }

void Menu::Update() {
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

	isPreTrigger_ = isTrigger_;
	if (PlayerCommand::GetInstance()->InteractTrigger()) {
		isTrigger_ = true;
	}
}

void Menu::Draw() {
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
	if (isPreTrigger_ && isTrigger_) {
	
	
	}

}