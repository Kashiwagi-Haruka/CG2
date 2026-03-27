#pragma once
#include <string>
#include "Option/Option.h"
#include <memory>
#include "Text/Text.h"
class Menu {

	std::string currentMenuName_ = "Game";

	bool isTrigger_ = false;
	bool isPreTrigger_ = false;

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

};
