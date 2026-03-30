#include "Mission.h"
#include "Engine/Texture/Data/Color.h"
#include "Sprite/SpriteCommon.h"
#include "Color/Color.h"
Mission::Mission() = default;
void Mission::Initialize() {	
	fontHandle_ = FreeTypeManager::CreateFace("Resources/TD3_3102/Irohakaku/irohakakuC-Medium.ttf", 0);
	FreeTypeManager::SetPixelSizes(fontHandle_, 24, 24);
	MissionText_.Initialize(fontHandle_);
	MissionText_.SetString(U"ミッション");
	MissionText_.SetPosition({50, 200});
	MissionText_.SetColor(Color::RGBAToVector4(180,180,50,255));
	MissionText_.SetAlign(TextAlign::Left);
	MissionText_.UpdateLayout(false);
}
void Mission::Update() {
	/*MissionText_.Update();*/
	MissionText_.UpdateLayout(false);
}
void Mission::Draw() {
	SpriteCommon::GetInstance()->DrawCommonFont();
	MissionText_.Draw(); }