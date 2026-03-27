#pragma once
#include "Option/Option.h"
#include "Text/Text.h"
#include <memory>
#include <string>
class Menu {
public:
	enum class Action { kNone, kResumeGame, kSave, kOpenOption, kBackToTitle, kEndGame };

private:
	std::string currentMenuName_ = "Game";

	bool isTrigger_ = false;
	bool isPreTrigger_ = false;
	Action pendingAction_ = Action::kNone;

	std::unique_ptr<Option> option_;

	Text menuText_;
	Text GameText_;
	Text SaveText_;
	Text OptionText_;
	Text TitleText_;
	Text GameEndText_;
	uint32_t fontHandle_ = 0;

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