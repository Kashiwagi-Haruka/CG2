#include "GameContinuedText.h"

void GameContinuedText::Initialize() {
	// フォントハンドル
	fontHandle_ = FreeTypeManager::CreateFace("Resources/TD3_3102/Irohakaku/irohakakuC-Medium.ttf", 0);
	FreeTypeManager::SetPixelSizes(fontHandle_, 64, 64);
	for (auto& saveDataText : gameSaveDataText) {
		saveDataText.Name_.Initialize(fontHandle_);
		saveDataText.currentStageName_.Initialize(fontHandle_);
		saveDataText.saveDateTime_.Initialize(fontHandle_);
	}
}
void GameContinuedText::Update() {}
void GameContinuedText::Draw() {
	for (auto& saveDataText : gameSaveDataText) {
		saveDataText.Name_.Draw();
		saveDataText.currentStageName_.Draw();
		saveDataText.saveDateTime_.Draw();
	}
}
void GameContinuedText::SetSaveDataText(const std::string& name, const std::string& currentStageName, const std::string& saveDateTime, int index) {
	if (index < 0 || index >= saveDataMaxNum_) {
		return;
	}
	gameSaveDataText[index].Name_.SetString(std::u32string(name.begin(), name.end()));
	gameSaveDataText[index].currentStageName_.SetString(std::u32string(currentStageName.begin(), currentStageName.end()));
	gameSaveDataText[index].saveDateTime_.SetString(std::u32string(saveDateTime.begin(), saveDateTime.end()));
}