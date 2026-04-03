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
		
		gameSaveDataText[i].Name_.Initialize(fontHandle_);
		gameSaveDataText[i].currentStageName_.Initialize(fontHandle_);
		gameSaveDataText[i].saveDateTime_.Initialize(fontHandle_);

		gameSaveDataText[i].Name_.SetAlign(TextAlign::Left);
		gameSaveDataText[i].currentStageName_.SetAlign(TextAlign::Left);
		gameSaveDataText[i].saveDateTime_.SetAlign(TextAlign::Left);

		gameSaveDataText[i].Name_.SetColor(COLOR::BLACK);
		gameSaveDataText[i].currentStageName_.SetColor(COLOR::BLACK);
		gameSaveDataText[i].saveDateTime_.SetColor(COLOR::BLACK);

		gameSaveDataText[i].Name_.SetSize({64.0f,64.0f});
		gameSaveDataText[i].currentStageName_.SetSize({64.0f, 64.0f});
		gameSaveDataText[i].saveDateTime_.SetSize({64.0f, 64.0f});

		gameSaveDataText[i].Name_.SetString(U"SaveFile");
		gameSaveDataText[i].currentStageName_.SetString(U"NoData");
		gameSaveDataText[i].saveDateTime_.SetString(U"1111");

		gameSaveDataText[i].Name_.UpdateLayout(false);
		gameSaveDataText[i].currentStageName_.UpdateLayout(false);
		gameSaveDataText[i].saveDateTime_.UpdateLayout(false);

		SetBlockLayout(i, {640.0f, 360.0f + (280.0f * static_cast<float>(i))}, {1040.0f, 240.0f});
	}

	selectionCursorText_.Initialize(fontHandle_);
	selectionCursorText_.SetString(U"▶");
	selectionCursorText_.SetAlign(TextAlign::Left);
	selectionCursorText_.SetColor(COLOR::RED);

	SetCurrentSelectIndex(0);
}
void GameContinuedText::Update(int selectIndex) { 
	SetCurrentSelectIndex(selectIndex); 


}

void GameContinuedText::Draw() {
	for (int i = 0; i < static_cast<int>(gameSaveDataText.size()); ++i) {
		gameSaveDataText[i].Name_.Draw();
		gameSaveDataText[i].currentStageName_.Draw();
		gameSaveDataText[i].saveDateTime_.Draw();
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


}
void GameContinuedText::SetSaveDataText(const std::string& name, const std::string& currentStageName, const std::string& saveDateTime, int index) {
	if (index < 0 || index >= saveDataMaxNum_) {
		return;
	}
	gameSaveDataText[index].Name_.SetString(std::u32string(name.begin(), name.end()));
	gameSaveDataText[index].currentStageName_.SetString(std::u32string(currentStageName.begin(), currentStageName.end()));
	gameSaveDataText[index].saveDateTime_.SetString(std::u32string(saveDateTime.begin(), saveDateTime.end()));

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