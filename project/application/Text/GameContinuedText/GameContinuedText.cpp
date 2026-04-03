#include "GameContinuedText.h"
#include "Color/Color.h"

namespace {
constexpr float kStartX = 220.0f;
constexpr float kStartY = 160.0f;
constexpr float kRowOffsetY = 100.0f;
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

		Vector2 linePosition = {kStartX, kStartY + kRowOffsetY * static_cast<float>(i)};
		saveDataText.Name_.SetPosition(linePosition);
		saveDataText.currentStageName_.SetPosition({linePosition.x + 320.0f, linePosition.y});
		saveDataText.saveDateTime_.SetPosition({linePosition.x + 700.0f, linePosition.y});

		saveDataText.Name_.SetAlign(TextAlign::Left);
		saveDataText.currentStageName_.SetAlign(TextAlign::Left);
		saveDataText.saveDateTime_.SetAlign(TextAlign::Left);

		saveDataText.Name_.SetColor(COLOR::WHITE);
		saveDataText.currentStageName_.SetColor(COLOR::WHITE);
		saveDataText.saveDateTime_.SetColor(COLOR::WHITE);

		SetSaveDataText("Save " + std::to_string(i + 1), "No Data", "--:--", i);
	}

	selectionCursorText_.Initialize(fontHandle_);
	selectionCursorText_.SetString(U"▶");
	selectionCursorText_.SetAlign(TextAlign::Left);
	selectionCursorText_.SetColor(COLOR::RED);

	SetCurrentSelectIndex(0);
}
void GameContinuedText::Update(int selectIndex) { SetCurrentSelectIndex(selectIndex); }

void GameContinuedText::Draw() {
	for (auto& saveDataText : gameSaveDataText) {
		saveDataText.Name_.Draw();
		saveDataText.currentStageName_.Draw();
		saveDataText.saveDateTime_.Draw();
	}
	selectionCursorText_.Draw();
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
	cursorPos.x -= 60.0f;
	selectionCursorText_.SetPosition(cursorPos);
}