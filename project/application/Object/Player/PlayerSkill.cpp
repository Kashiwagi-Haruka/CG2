#include "PlayerSkill.h"
#include "GameBase.h"
#include "Object3dCommon.h"
PlayerSkill::PlayerSkill() {
	
}
void PlayerSkill::Initialize() {
	debugBox_ = std::make_unique<Object3d>();
	debugBox_->Initialize(GameBase::GetInstance()->GetObject3dCommon());
	debugBox_->SetCamera(camera_);
	debugBox_->SetModel("debugBox");
	debugDamageBox1_ = std::make_unique<Object3d>();
	debugDamageBox1_->Initialize(GameBase::GetInstance()->GetObject3dCommon());
	debugDamageBox1_->SetCamera(camera_);
	debugDamageBox1_->SetModel("debugBox");
	debugDamageBox2_ = std::make_unique<Object3d>();
	debugDamageBox2_->Initialize(GameBase::GetInstance()->GetObject3dCommon());
	debugDamageBox2_->SetCamera(camera_);
	debugDamageBox2_->SetModel("debugBox");
	skillUpObject_ = std::make_unique<Object3d>();
	skillUpObject_->Initialize(GameBase::GetInstance()->GetObject3dCommon());
	skillUpObject_->SetCamera(camera_);
	skillUpObject_->SetModel("playerSkillUp");

	skillUnderObject_ = std::make_unique<Object3d>();
	skillUnderObject_->Initialize(GameBase::GetInstance()->GetObject3dCommon());
	skillUnderObject_->SetCamera(camera_);
	skillUnderObject_->SetModel("playerSkillUnder");
	transform_ = {
		.scale{2.0f, 2.0f, 2.0f},
		.rotate{0.0f, 0.0f, 0.0f},
		.translate{0.0f, 0.0f, 0.0f}
    };
	damageTransform1_ = {
	    .scale{1.5f, 1.5f, 1.5f},
        .rotate{0.0f, 0.0f, 0.0f},
        .translate{0.0f, 0.0f, 0.0f}
    };
	damageTransform2_ = {
	    .scale{0.0f, 0.0f,0.0f},
        .rotate{0.0f, 0.0f, 0.0f},
        .translate{0.0f, 0.0f, 0.0f}
    };
	particle_ = {
	    .scale{1, 1, 1},
        .rotate{0, 0, 0},
        .translate{0, 0, 0}
    };
	skillEmitter_ = std::make_unique<ParticleEmitter>("skill", particle_, 10.0f, 1, Vector3{0, 1, 0}, Vector3{-transform_.scale.x / 2.0f, 0, -transform_.scale.z / 2.0f}, Vector3{transform_.scale.x / 2.0f, 0, transform_.scale.z / 2.0f});
}
void PlayerSkill::Update() {
	

	switch (state) {
	case PlayerSkill::up:
		damageTransform1_.translate.y = Function::Lerp(transform_.translate.y, downstartposY, upTime);
		if (upTime < 1.0f) {
			upTime += 0.1f;
		} else {
			state = middle;
		}
		break;
	case PlayerSkill::middle:
		damageTransform1_.rotate.y += 0.5f;
		if (middleTime < 1.0f) {
			middleTime += 0.1f;
		} else {
			state = down;
		}

		break;
	case PlayerSkill::down:
		damageTransform1_.translate.y = Function::Lerp(downstartposY, 2.5f, downTime);
		if (downTime < 1.0f) {
			downTime += 0.1f;
		} else {
			state = damage;	
		}
		break;
	case PlayerSkill::damage:
		damageTransform2_.translate.y = 0.0f;
		damageTransform2_.scale.x = Function::Lerp(0, 3, damageTime);
		damageTransform2_.scale.y = Function::Lerp(0, 3, damageTime);
		damageTransform2_.scale.z = Function::Lerp(0, 3, damageTime);
		if (damageTime < 1.0f) {
			damageTime += 0.1f;
		} else {
			if (endTime < 30.0f) {
				endTime++;
			} else {
				isSkillEnd = true;
			}
		}
		break;
	default:
		break;
	}

	
	debugBox_->SetCamera(camera_);
	debugBox_->SetTransform(transform_);
	debugBox_->Update();
	debugDamageBox1_->SetCamera(camera_);
	debugDamageBox1_->SetTransform(damageTransform1_);
	debugDamageBox1_->Update();
	
	debugDamageBox2_->SetCamera(camera_);
	debugDamageBox2_->SetTransform(damageTransform2_);
	debugDamageBox2_->Update();

	skillUpObject_->SetCamera(camera_);
	skillUpObject_->SetTransform(damageTransform1_);
	skillUpObject_->Update();

	skillUnderObject_->SetCamera(camera_);
	skillUnderObject_->SetTransform(damageTransform2_);
	skillUnderObject_->Update();
	
	skillEmitter_->Update(particle_);
}
void PlayerSkill::StartAttack(const Transform& playerTransform) {
	transform_ = playerTransform;
	damageTransform1_ = playerTransform;
	damageTransform2_ = playerTransform;
	particle_.translate = {transform_.translate.x, 0, transform_.translate.z};
	skillUnderObject_->SetTransform(damageTransform2_);
	damageTransform2_.translate.y = -5.0f;
	isSkillEnd = false;
	skillTime = 0;
	state = up;
	upTime = 0;
	middleTime = 0;
	downTime = 0;
	damageTime = 0;
	endTime = 0;
}
void PlayerSkill::Draw() {
	
	debugBox_->Draw(); 
	debugDamageBox1_->Draw();
	debugDamageBox2_->Draw();
	GameBase::GetInstance()->GetObject3dCommon()->SetBlendMode(BlendMode::kBlendScreen);
	skillUpObject_->Draw();
	skillUnderObject_->Draw();
	GameBase::GetInstance()->GetObject3dCommon()->SetBlendMode(BlendMode::kBlendModeAlpha);
}



