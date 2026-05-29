#pragma once

#include "Text/Text.h"
#include <memory>
#include "Sprite.h"

class TextUIManager {
public:
	TextUIManager();
	~TextUIManager();
	void Initialize();
	void Update();
	void Draw();
	bool GetTalkTyping() { return gentlemanTalk_.GetIsTyping()|| edamameTrivia_.GetIsTyping()|| coffeeTrivia_.GetIsTyping(); };

public:
	void ShowKeyLostAtStageStartMessage();

private:
	void StartText(float showTime = 1.0f);

private:
	uint32_t fontHandle_;
	Text text_;
	Text edamameTrivia_;
	Text coffeeTrivia_;
	//紳士テキスト
	Text gentlemanTalk_;
	float spriteAlpha_ = 0.0f;
	std::unique_ptr<Sprite> sprite_ = nullptr;

	float showTime_ = 1.0f;
	float showTimer_ = showTime_;
	bool isDraw_ = false;

	float edamameTriviaAlpha_ = 0.0f;
	float coffeeTriviaAlpha_ = 0.0f;
	float gentlemanAlpha_ = 0.0f;
	bool wasEdamameRayHit_ = false;
	bool wasCoffeeRayHit_ = false;
	static constexpr float kEdamameFadeDuration_ = 3.0f;
};