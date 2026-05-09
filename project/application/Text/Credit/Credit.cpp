#include "Credit.h"

#include "Engine/Loadfile/TXT/TxtManager.h"
#include "application/Color/Color.h"
#include"ScreenSize/ScreenSize.h"

void Credit::Initialize(uint32_t fontHandle) {
	creditBodyText_.Initialize(fontHandle);
	creditBodyText_.SetPosition({ SCREEN_SIZE::HALF_WIDTH,256
		});
	creditBodyText_.SetColor(COLOR::WHITE);
	creditBodyText_.SetAlign(TextAlign::Center);

	try {
		creditBodyText_.SetString(TxtManager::GetInstance()->LoadTxtAsU32String("Resources/TXT/Credit.txt"));
	} catch (...) {
		creditBodyText_.SetString(U"Credit.txt の読み込みに失敗しました。");
	}

	creditBodyText_.UpdateLayout(false);
}

void Credit::Draw() { creditBodyText_.Draw(); }