#define NOMINMAX
#include "GameContinuedText.h"
#include "Color/Color.h"
#include <algorithm>

namespace {
constexpr float kCursorOffsetX = 54.0f;
} // namespace
void GameContinuedText::Initialize() {
	// フォントハンドル
	fontHandle_ = FreeTypeManager::CreateFace("Resources/TD3_3102/Irohakaku/irohakakuC-Medium.ttf", 0);
	FreeTypeManager::SetPixelSizes(fontHandle_, 32, 32);
	gameSaveDataText_.resize(saveDataMaxNum_);
	for (int i = 0; i < static_cast<int>(gameSaveDataText_.size()); ++i) {

		gameSaveDataText_[i].Name_.Initialize(fontHandle_);
		gameSaveDataText_[i].currentStageName_.Initialize(fontHandle_);
		gameSaveDataText_[i].saveDateTime_.Initialize(fontHandle_);

		gameSaveDataText_[i].Name_.SetAlign(TextAlign::Left);
		gameSaveDataText_[i].currentStageName_.SetAlign(TextAlign::Left);
		gameSaveDataText_[i].saveDateTime_.SetAlign(TextAlign::Left);

		gameSaveDataText_[i].Name_.SetBlendMode(BlendMode::kBlendModeMultipy);
		gameSaveDataText_[i].currentStageName_.SetBlendMode(BlendMode::kBlendModeMultipy);
		gameSaveDataText_[i].saveDateTime_.SetBlendMode(BlendMode::kBlendModeMultipy);

		gameSaveDataText_[i].Name_.SetColor(COLOR::BLACK);
		gameSaveDataText_[i].currentStageName_.SetColor(COLOR::BLACK);
		gameSaveDataText_[i].saveDateTime_.SetColor(COLOR::BLACK);

		gameSaveDataText_[i].Name_.SetSize({200.0f, 200.0f});
		gameSaveDataText_[i].currentStageName_.SetSize({200.0f, 200.0f});
		gameSaveDataText_[i].saveDateTime_.SetSize({200.0f, 200.0f});




		SetBlockLayout(i, {640.0f, 360.0f + (280.0f * static_cast<float>(i))}, {1040.0f, 240.0f});
	}

	selectionCursorText_.Initialize(fontHandle_);
	selectionCursorText_.SetString(U"▶");
	selectionCursorText_.SetAlign(TextAlign::Left);
	selectionCursorText_.SetColor(COLOR::RED);

	SetCurrentSelectIndex(0);
}
void GameContinuedText::Update(int selectIndex) { SetCurrentSelectIndex(selectIndex); 
	for (auto& saveDataText : gameSaveDataText_) {
		saveDataText.Name_.UpdateLayout(false);
		saveDataText.currentStageName_.UpdateLayout(false);
		saveDataText.saveDateTime_.UpdateLayout(false);
	}
}

void GameContinuedText::Draw() {
	for (auto& saveDataText : gameSaveDataText_) {
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
	const float right = blockCenter.x + (blockScale.x * 0.5f);
	const float top = blockCenter.y - (blockScale.y * 0.5f);

	const float thumbnailHeight = std::max(1.0f, blockScale.y - 24.0f);
	const float thumbnailWidth = thumbnailHeight * (9.0f / 16.0f);
	const float textAreaLeft = left + 30.0f + thumbnailWidth + 40.0f;
	const float textAreaRight = right - 48.0f;
	const float nameX = std::max(textAreaLeft, textAreaRight - 560.0f);
	const float stageX = std::max(textAreaLeft, textAreaRight - 340.0f);
	const float dateX = std::max(textAreaLeft, textAreaRight - 140.0f);
	const float textY = top + (blockScale.y * 0.5f);

	gameSaveDataText_[index].Name_.SetPosition({nameX, textY});
	gameSaveDataText_[index].currentStageName_.SetPosition({stageX, textY});
	gameSaveDataText_[index].saveDateTime_.SetPosition({dateX, textY});
}
void GameContinuedText::SetSaveDataText(const std::string& name, const std::string& currentStageName, const std::string& saveDateTime, int index) {
	if (index < 0 || index >= saveDataMaxNum_) {
		return;
	}
	gameSaveDataText_[index].Name_.SetString(std::u32string(name.begin(), name.end()));
	gameSaveDataText_[index].currentStageName_.SetString(std::u32string(currentStageName.begin(), currentStageName.end()));
	gameSaveDataText_[index].saveDateTime_.SetString(std::u32string(saveDateTime.begin(), saveDateTime.end()));
}

void GameContinuedText::SetCurrentSelectIndex(int index) {
	if (index < 0 || index >= saveDataMaxNum_) {
		return;
	}

	currentSelectIndex_ = index;

	Vector2 cursorPos = gameSaveDataText_[currentSelectIndex_].Name_.GetPosition();
	cursorPos.x -= kCursorOffsetX;
	selectionCursorText_.SetPosition(cursorPos);
	selectionCursorText_.UpdateLayout(false);
}