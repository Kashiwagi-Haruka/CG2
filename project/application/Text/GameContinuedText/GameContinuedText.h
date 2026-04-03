#pragma once
#include <array>
#include "GameObject/UI/GameContinued/GameContinuedData.h"
#include "Text/Text.h"
class GameContinuedText {

	struct SaveDataText {
		Text Name_;                        
		Text currentStageName_;            
		Text saveDateTime_;                
	};

	std::array<SaveDataText, saveDataMaxNum_> gameSaveDataText;	
	uint32_t fontHandle_ = 0;

	public:
	void Initialize();
	void Update();
	void Draw();

	void SetSaveDataText(const std::string& name, const std::string& currentStageName, const std::string& saveDateTime, int index);

};
