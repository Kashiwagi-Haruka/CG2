#include "PlayerSkill.h"
#include "GameBase.h"
PlayerSkill::PlayerSkill() {

}
void PlayerSkill::Initialize() {
	debugBox_ = std::make_unique<Object3d>();
	debugBox_->Initialize(GameBase::GetInstance()->GetObject3dCommon());
	debugBox_->SetCamera(camera_);
	debugBox_->SetModel("debugBox");
	transform_ = {
		.scale{2.0f, 2.0f, 2.0f},
		.rotate{0.0f, 0.0f, 0.0f},
		.translate{0.0f, 0.0f, 0.0f}
    };
}
void PlayerSkill::Update(const Transform& playerTransform) {
	transform_ = playerTransform;
	
	debugBox_->SetColor({0.0f, 0.0f, 1.0f, 0.5f});
	debugBox_->SetCamera(camera_);
	debugBox_->SetTransform(transform_);
	debugBox_->Update();
}
void PlayerSkill::StartAttack() { isSkillAttack = true; }
void PlayerSkill::Draw() {
	if (isSkillAttack) {
	debugBox_->Draw(); 
	}
}



