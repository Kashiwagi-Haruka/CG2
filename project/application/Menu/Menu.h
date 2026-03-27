#pragma once
#include <string>
#include "Option/Option.h"

class Menu {

	std::string currentMenuName_ = "Game";

	bool isTrigger_ = false;
	bool isPreTrigger_ = false;


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
