#pragma once

#include "Audio.h"
#include "GameObject/YoshidaMath/RandomClass.h"
#include "Text/Text.h"
#include <array>
#include <memory>

class TitleMenuUI {
public:
	// テキストの設定
	TitleMenuUI();
	~TitleMenuUI();
	void Initialize();
	void Update();
	void Draw();
	bool GetIsStart() { return isInitStart_; }
	bool ConsumeContinueTriggered();

private:
#pragma region // Text

	enum MENU {
		START_TEXT,
		CONTINUE_TEXT,
		OPTION_TEXT,
		MAX_TEXT,
	};

	uint32_t fontHandle_;
	uint32_t menuFontHandle_;

	Text titleText_;
	Text triangleText_;
	Text pressSpaceText_;
	std::array<Text, MAX_TEXT> menuText_;
	float fontTheta_ = 0.0f;

	std::unique_ptr<RandomClass> random_ = nullptr;
	Vector2 titleDefaultPos_ = {0.0f};
#pragma endregion

	bool isShowMenu_ = false;
	//初めからスタート
	bool isInitStart_ = false;
	//続きから
	bool isContinueTriggered_ = false;
	uint32_t selectButtonNum_ = START_TEXT;
};