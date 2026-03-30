#include "Mission.h"
#include "Color/Color.h"
#include "Engine/Texture/Data/Color.h"
#include "GameObject/Key/Key.h"
#include "Sprite/SpriteCommon.h"
Mission::Mission() = default;
void Mission::Initialize() {
	fontHandle_ = FreeTypeManager::CreateFace("Resources/TD3_3102/Irohakaku/irohakakuC-Medium.ttf", 0);
	FreeTypeManager::SetPixelSizes(fontHandle_, 30, 30);

	MissionText_.Initialize(fontHandle_);
	MissionText_.SetString(U"ミッション");
	MissionText_.SetPosition({50, 200});
	MissionText_.SetColor(Color::RGBAToVector4(180, 180, 50, 255));
	MissionText_.SetAlign(TextAlign::Left);
	MissionText_.UpdateLayout(false);

	KeyGetMissionText_.Initialize(fontHandle_);
	KeyGetMissionText_.SetString(U"鍵を探せ");
	KeyGetMissionText_.SetPosition({50, 260});
	KeyGetMissionText_.SetColor(Color::RGBAToVector4(255, 255, 255, 255));
	KeyGetMissionText_.SetAlign(TextAlign::Left);
	KeyGetMissionText_.UpdateLayout(false);

	GotoDoorMissionText_.Initialize(fontHandle_);
	GotoDoorMissionText_.SetString(U"ドアへ向かえ");
	GotoDoorMissionText_.SetPosition({50, 260});
	GotoDoorMissionText_.SetColor(Color::RGBAToVector4(255, 255, 255, 255));
	GotoDoorMissionText_.SetAlign(TextAlign::Left);
	GotoDoorMissionText_.UpdateLayout(false);
}
void Mission::Update() {
	/*MissionText_.Update();*/
	MissionText_.UpdateLayout(false);
	KeyGetMissionText_.UpdateLayout(false);
	GotoDoorMissionText_.UpdateLayout(false);
}
void Mission::Draw() {
	SpriteCommon::GetInstance()->DrawCommonFont();
	MissionText_.Draw();

	if (Key::IsGetKey()) {
		GotoDoorMissionText_.Draw();
	} else {
		KeyGetMissionText_.Draw();
	}
}