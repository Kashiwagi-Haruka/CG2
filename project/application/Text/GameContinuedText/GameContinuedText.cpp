#define NOMINMAX
#include "GameContinuedText.h"
#include "Color/Color.h"
#include <algorithm>

namespace {
constexpr float kTextLeftPadding = 40.0f;
constexpr float kTextGapX = 270.0f;
constexpr float kDateGapX = 560.0f;
constexpr float kCursorOffsetX = 54.0f;
} // namespace

void GameContinuedText::Initialize() {
	// フォントハンドル
	fontHandle_ = FreeTypeManager::CreateFace("Resources/TD3_3102/Irohakaku/irohakakuC-Medium.ttf", 0);
	FreeTypeManager::SetPixelSizes(fontHandle_, 64, 64);
	for (int i = 0; i < static_cast<int>(gameSaveDataText.size()); ++i) {
		auto& saveDataText = gameSaveDataText[i];
		saveDataText.Name_.Initialize(fontHandle_);
		saveDataText.currentStageName_.Initialize(fontHandle_);
		saveDataText.saveDateTime_.Initialize(fontHandle_);

		saveDataText.Name_.SetAlign(TextAlign::Left);
		saveDataText.currentStageName_.SetAlign(TextAlign::Left);
		saveDataText.saveDateTime_.SetAlign(TextAlign::Left);

		saveDataText.Name_.SetColor(COLOR::BLACK);
		saveDataText.currentStageName_.SetColor(COLOR::BLACK);
		saveDataText.saveDateTime_.SetColor(COLOR::BLACK);

		saveDataText.Name_.SetSize({64.0f,64.0f});
		saveDataText.currentStageName_.SetSize({64.0f, 64.0f});
		saveDataText.saveDateTime_.SetSize({64.0f, 64.0f});

		SetBlockLayout(i, {640.0f, 360.0f + (280.0f * static_cast<float>(i))}, {1040.0f, 240.0f});

		SetSaveDataText("Save " + std::to_string(i + 1), "No Data", "--:--", i);
	}

	selectionCursorText_.Initialize(fontHandle_);
	selectionCursorText_.SetString(U"▶");
	selectionCursorText_.SetAlign(TextAlign::Left);
	selectionCursorText_.SetColor(COLOR::RED);

	SetCurrentSelectIndex(0);
}
void GameContinuedText::Update(int selectIndex) { 
	SetCurrentSelectIndex(selectIndex); 
		for (int i = 0; i < static_cast<int>(gameSaveDataText.size()); ++i) {
		auto& saveDataText = gameSaveDataText[i];
		saveDataText.Name_.Update();
		saveDataText.currentStageName_.Update();
		saveDataText.saveDateTime_.Update();
	}
}

void GameContinuedText::Draw() {
	for (auto& saveDataText : gameSaveDataText) {
		saveDataText.Name_.Draw();
		saveDataText.currentStageName_.Draw();
		saveDataText.saveDateTime_.Draw();
	}
	selectionCursorText_.Draw();
}

void GameContinuedText::SetBlockLayout(int index, const Vector2& blockCenter, const Vector2& blockScale) {
	if (index < 0 || index >= saveDataMaxNum_) {
		return;
	}

	const float left = blockCenter.x - (blockScale.x * 0.5f);
	const float top = blockCenter.y - (blockScale.y * 0.5f);
	const float thumbnailHeight = std::max(1.0f, blockScale.y - 24.0f);
	const float thumbnailWidth = thumbnailHeight * (9.0f / 16.0f);
	const float textBaseX = left + 30.0f + thumbnailWidth + kTextLeftPadding;
	const float textY = top + (blockScale.y * 0.5f);

	gameSaveDataText[index].Name_.SetPosition({textBaseX, textY});
	gameSaveDataText[index].currentStageName_.SetPosition({textBaseX + kTextGapX, textY});
	gameSaveDataText[index].saveDateTime_.SetPosition({textBaseX + kDateGapX, textY});
	gameSaveDataText[index].Name_.UpdateLayout(false);
	gameSaveDataText[index].currentStageName_.UpdateLayout(false);
	gameSaveDataText[index].saveDateTime_.UpdateLayout(false);
}
void GameContinuedText::SetSaveDataText(const std::string& name, const std::string& currentStageName, const std::string& saveDateTime, int index) {
	if (index < 0 || index >= saveDataMaxNum_) {
		return;
	}
	gameSaveDataText[index].Name_.SetString(std::u32string(name.begin(), name.end()));
	gameSaveDataText[index].currentStageName_.SetString(std::u32string(currentStageName.begin(), currentStageName.end()));
	gameSaveDataText[index].saveDateTime_.SetString(std::u32string(saveDateTime.begin(), saveDateTime.end()));
	gameSaveDataText[index].Name_.UpdateLayout(false);
	gameSaveDataText[index].currentStageName_.UpdateLayout(false);
	gameSaveDataText[index].saveDateTime_.UpdateLayout(false);
}

void GameContinuedText::SetCurrentSelectIndex(int index) {
	if (index < 0 || index >= saveDataMaxNum_) {
		return;
	}

	currentSelectIndex_ = index;

	for (int i = 0; i < static_cast<int>(gameSaveDataText.size()); ++i) {
		Vector4 color = (i == currentSelectIndex_) ? COLOR::RED : COLOR::WHITE;
		gameSaveDataText[i].Name_.SetColor(color);
		gameSaveDataText[i].currentStageName_.SetColor(color);
		gameSaveDataText[i].saveDateTime_.SetColor(color);
	}

	Vector2 cursorPos = gameSaveDataText[currentSelectIndex_].Name_.GetPosition();
	cursorPos.x -= kCursorOffsetX;
	selectionCursorText_.SetPosition(cursorPos);
	selectionCursorText_.UpdateLayout(false);
}