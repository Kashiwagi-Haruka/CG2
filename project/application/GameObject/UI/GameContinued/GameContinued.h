#pragma once
#include <memory>
#include <array>
#include "Sprite/Sprite.h"
#include "GameContinuedData.h"
#include "Text/GameContinuedText/GameContinuedText.h"
class GameContinued {

	struct SaveData {
		std::unique_ptr<Sprite> GameSceneSprite_; // ゲームシーンのスクリーンショットを表示するスプライト
		std::unique_ptr<Sprite> BlockSprite_;     // セーブデータブロックのスクリーンショットを表示するスプライト
		std::string Name_;                        // セーブデータの名前
		std::string currentStageName_;            // セーブデータの現在のステージ名
		std::string saveDateTime_;                // セーブデータの保存日時
	};

	std::array<SaveData,saveDataMaxNum_> gameSaveData;
	int currentSelectNum_ = 0;

	std::unique_ptr<GameContinuedText> text_;

	public:

	void Initialize();
	void Update();
	void Draw();


};
