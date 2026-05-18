#include "EleveterNumberText.h"

#include "Text/FreetypeManager/FreeTypeManager.h"

EleveterNumberText::EleveterNumberText() {
	fontHandle_ = FreeTypeManager::CreateFace("Resources/TD3_3102/Irohakaku/irohakakuC-Bold.ttf", 0);
	FreeTypeManager::SetPixelSizes(fontHandle_, 32, 32);
	text_.Initialize(fontHandle_);
	text_.SetPosition({640.0f, 96.0f});
	text_.SetAlign(TextAlign::Center);
	text_.SetBlendMode(BlendMode::kBlendModeAlpha);
	text_.SetColor({1.0f, 1.0f, 1.0f, 1.0f});
	SetStageNumber(0);
}

void EleveterNumberText::SetStageNumber(int stageNumber) {
	const std::string numberText = std::to_string(stageNumber);
	text_.SetString(std::u32string(numberText.begin(), numberText.end()));
	text_.UpdateLayout(false);
}

void EleveterNumberText::Update() { text_.Update(); }

void EleveterNumberText::Draw() { text_.Draw(); }