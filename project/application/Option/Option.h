#pragma once
#include "OptionData.h"
#include "Sprite/Sprite.h"
#include <array>
class Option {

	bool isShowOption_ = false;
	OptionData optionData_;
	static const int kOptionParameterDivisionNum = 10; // オプションのパラメータの分割数
	static const int kOptionParameterNum = 4;          // オプションのパラメータの数
	std::array<std::array<Sprite,kOptionParameterDivisionNum>,kOptionParameterNum> parameterSprite_;
	public:

	void OpenOption() { isShowOption_ = true;}
	void CloseOption() { isShowOption_ = false; }
	bool GetIsShowOption() const { return isShowOption_; }

	void Initialize();
	void Update();
	void Draw();

	void SaveOptionData();
	void LoadOptionData();
};
