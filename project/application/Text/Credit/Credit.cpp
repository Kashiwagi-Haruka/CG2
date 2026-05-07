#include "Credit.h"

#include "Engine/Loadfile/TXT/TxtManager.h"
#include "application/Color/Color.h"

void Credit::Initialize(uint32_t fontHandle) {
	creditBodyText_.Initialize(fontHandle);
	creditBodyText_.SetPosition({40.0f, 80.0f});
	creditBodyText_.SetColor(COLOR::WHITE);
	creditBodyText_.SetAlign(TextAlign::Left);

	try {
		creditBodyText_.SetString(TxtManager::GetInstance()->LoadTxtAsU32String("Resources/TXT/Credit.txt"));
	} catch (...) {
		creditBodyText_.SetString(U"Credit.txt の読み込みに失敗しました。");
	}

	creditBodyText_.UpdateLayout(false);
}

void Credit::Draw() { creditBodyText_.Draw(); }