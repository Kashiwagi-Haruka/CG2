#pragma once
#include "Option/Option.h"
#include "Sprite/Sprite.h"
#include "Text/Text.h"
#include <memory>
#include <string>
class Menu {
public:
	enum class Action { kNone, kResumeGame, kCredit, kOpenOption, kBackToTitle, kEndGame };

private:
	std::string currentMenuName_ = "Game";

	bool isTrigger_ = false;
	bool isPreTrigger_ = false;
	Action pendingAction_ = Action::kNone;

	std::unique_ptr<Option> option_;

	Text menuText_;
	Text GameText_;
	Text CreditText_;
	Text OptionText_;
	Text TitleText_;
	Text GameEndText_;
	Text tabBackHintText_;
	Text confirmHintText_;
	uint32_t fontHandle_ = 0;
	std::unique_ptr<Sprite> backgroundOverlaySprite_{};

public:
	/// <summary>
	/// "Game"ゲームに戻る,"Save"セーブ,"Option"オプション,"Title"タイトル,"GameEnd"ゲームを終わる
	/// </summary>
	/// <param name="MenuName"></param>
	void ChangeMenu(std::string MenuName);

	void Initialize();
	void Update();
	void Draw();
	Action ConsumePendingAction();
	bool IsOptionOpen() const;
	void CloseOptionAndPrepareResume();
};