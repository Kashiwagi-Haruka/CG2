#pragma once
#include "GameObject/UI/GameContinued/GameContinuedData.h"
#include "Text/Text.h"
#include <vector>
class GameContinuedText {

	struct SaveDataText {
		Text Name_;
		Text currentStageName_;
		Text saveDateTime_;
	};

	std::vector<SaveDataText> gameSaveDataText_;
	Text selectionCursorText_;
	uint32_t fontHandle_ = 0;
	int currentSelectIndex_ = 0;

public:
	void Initialize();
	void Update(int selectIndex);
	void Draw();
	void SetBlockLayout(int index, const Vector2& blockCenter, const Vector2& blockScale);

	void SetSaveDataText(const std::string& name, const std::string& currentStageName, const std::string& saveDateTime, int index);
	void SetCurrentSelectIndex(int index);
};