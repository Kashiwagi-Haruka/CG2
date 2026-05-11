#pragma once
#include "Text/ChairMenu/ChairMenu.h"
#include "Text/Text.h"
#include <memory>

class TextUIManager {
public:
	TextUIManager();
	~TextUIManager();
	void Initialize();
	void Update();
	void Draw();

public:
	void ShowKeyLostAtStageStartMessage();

private:
	void StartText(float showTime = 1.0f);

private:
	uint32_t fontHandle_;
	Text text_;
	Text edamameTrivia_;
	float showTime_ = 1.0f;
	float showTimer_ = showTime_;
	bool isDraw_ = false;

	float edamameTriviaAlpha_ = 0.0f;
	bool wasEdamameRayHit_ = false;
	static constexpr float kEdamameFadeDuration_ = 3.0f;
};