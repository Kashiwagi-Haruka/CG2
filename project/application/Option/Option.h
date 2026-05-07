#pragma once
#include "OptionData.h"
#include "Sprite/Sprite.h"
#include "Text/Text.h"
#include <array>
class Option {

	bool isShowOption_ = false;
	OptionData optionData_;
	int selectedParameterIndex_ = 0;
	static const int kOptionParameterDivisionNum = 10; // オプションのパラメータの分割数
	static const int kOptionParameterNum = 6;          // オプションのパラメータの数

	const Vector2 kOptionSpriteStartPos = {640.0f, 220.0f};
	const Vector2 kOptionSpriteStep = {52.0f, 64.0f};
	const Vector2 kOptionSpriteSize = {40.0f, 24.0f};
	const Vector2 kOptionLabelPos = {200.0f, 250.0f};
	const float kOptionLabelStepY = 64.0f;

	uint32_t fontHandle_ = 0;
	Text optionTitleText_;
	Text saveHintText_;
	std::array<Text, kOptionParameterNum> optionParameterTexts_;
	const std::array<std::u32string, kOptionParameterNum> kParameterLabels_ = {
	    U"カメラの感度設定",
		U"視野角(FOV)",        
		U"カメラの左右反転",  
	    U"BGMボリューム",
	    U"SEボリューム",
	    U"Voiceボリューム",
	};

	std::array<std::array<Sprite, kOptionParameterDivisionNum>, kOptionParameterNum> parameterSprite_;

public:
	static const OptionData& GetCurrentOptionData() { return currentOptionData_; }

	void OpenOption() { isShowOption_ = true; }
	void CloseOption() { isShowOption_ = false; }
	bool GetIsShowOption() const { return isShowOption_; }

	void Initialize();
	void Update();
	void Draw();

	void SaveOptionData();
	void LoadOptionData();

private:
	static OptionData currentOptionData_;
};