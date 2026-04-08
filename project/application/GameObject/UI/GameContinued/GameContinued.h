#pragma once
#include "GameContinuedData.h"
#include "Sprite/Sprite.h"
#include "Text/GameContinuedText/GameContinuedText.h"
#include <array>
#include <memory>
#include <string>

class GameContinued {

private:

	struct SaveData {
		std::unique_ptr<Sprite> GameSceneSprite_; // ゲームシーンのスクリーンショットを表示するスプライト
		std::unique_ptr<Sprite> BlockSprite_;     // セーブデータブロックを表示するスプライト
		std::string Name_;                        // セーブデータの名前
		std::string currentStageName_;            // セーブデータの現在のステージ名
		std::string saveDateTime_;                // セーブデータの保存日時
	};

	std::array<SaveData, saveDataMaxNum_> gameSaveData_{};
	std::array<Vector4, saveDataMaxNum_> blockColors_{};
	std::array<Vector2, saveDataMaxNum_> blockScales_{};
	std::array<Vector2, saveDataMaxNum_> blockPositions_{};

	int currentSelectNum_ = 0;
	bool isSelected_ = false;

	std::unique_ptr<GameContinuedText> text_;

public:
	void Initialize();
	void Update();
	void Draw();
	void SetSaveData(int index, const std::string& name, const std::string& currentStageName, const std::string& saveDateTime);
	int GetCurrentSelectNum() const { return currentSelectNum_; }
	bool GetIsSelected() const { return isSelected_; }
};
