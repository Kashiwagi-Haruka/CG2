#include "Mission.h"
#include "Engine/Texture/Data/Color.h"
#include "Sprite/SpriteCommon.h"
void Mission::Initialize() {	
	fontHandle_ = FreeTypeManager::CreateFace("Resources/TD3_3102/Irohakaku/irohakakuC-Medium.ttf", 0);
	MissionText_.Initialize(fontHandle_);
	MissionText_.SetString(U"ミッション");
	MissionText_.SetPosition({20, 300});
	MissionText_.SetColor(Color::RGBAToVector4(180,180,50,255));
	MissionText_.SetAlign(TextAlign::Left);
	MissionText_.UpdateLayout(false);
}
void Mission::Update() {}
void Mission::Draw() {
	SpriteCommon::GetInstance()->DrawCommonFont();
	MissionText_.Draw(); }