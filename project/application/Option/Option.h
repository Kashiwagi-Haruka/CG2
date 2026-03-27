#pragma once
#include "OptionData.h"
class Option {

	bool isShowOption_ = false;
	OptionData optionData_;

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
