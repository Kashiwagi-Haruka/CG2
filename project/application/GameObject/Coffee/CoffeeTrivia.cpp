#include "CoffeeTrivia.h"
#include "Camera.h"
#include "Function.h"
#include "GameObject/GameCamera/PlayerCamera/PlayerCamera.h"
#include "GameObject/KeyBindConfig.h"
#include "Model/ModelManager.h"
#include "Object3d/Object3dCommon.h"
#include "Option/Option.h"

bool CoffeeTrivia::isRayHit_ = false;
size_t CoffeeTrivia::triviaNum_ = 0;
std::array<std::u32string, 6> CoffeeTrivia::strings_;
bool CoffeeTrivia::isSendStartTriviaMessage_ = false;
bool CoffeeTrivia::isCurrentVoiceFinished_ = true;

CoffeeTrivia::CoffeeTrivia() {

	SetVol(Option::GetCurrentOptionData().VoiceVolume);
	strings_[0] = U"カフェインは、脳を刺激して頭痛や眠気を和らげ、腎臓に作用して尿量を増やし、心臓を刺激して心拍数を上げる働きがあります。";
	strings_[1] = U"例えばコーヒー1杯、150mLには約90mgのカフェインが含まれています。";
	strings_[2] = U"しかし、短時間に5g以上摂取すると重症化し、特に脳と心臓に深刻な影響が出ます。";
	strings_[3] = U"脳では吐き気、頭痛、手足の震え、重症になると興奮、錯乱、幻覚や妄想、痙攣発作などが起きたりします。";
	strings_[4] = U"心臓では重症になると心拍数が140回/分を超えて血圧が低下し、危険な不整脈から心停止に至ることがあります。";
	strings_[5] = U"そのため、カフェインは短時間に大量摂取せず、適量を間隔をあけて摂ることが重要です。";

	ModelManager::GetInstance()->LoadModel("Resources/TD3_3102/3d/Coffee_billboard", "CoffeeBillboard");
}

CoffeeTrivia::~CoffeeTrivia() { Audio::GetInstance()->SoundUnload(&triviaVoice_); }

void CoffeeTrivia::Initialize() {
	triviaNum_ = strings_.size() - 1;
	triviaObj_ = std::make_unique<Object3d>();
	triviaObj_->Initialize();
	triviaObj_->SetModel("CoffeeBillboard");
	triviaObj_->SetOutlineColor({1.0f, 1.0f, 0.0f, 1.0f});
	triviaObj_->SetOutlineWidth(10.0f);
	localAABB_ = {
	    .min = {-1.00f, -0.10f, -1.00f},
          .max = {1.00f,  1.10f,  1.00f }
    };
	isActive_ = false;
	isLanded_ = false;
	isRayHit_ = false;
}

void CoffeeTrivia::Update() {
	isSendStartTriviaMessage_ = false;
	isCurrentVoiceFinished_ = Audio::GetInstance()->IsSoundFinished(triviaVoice_);
	if (!isActive_) {
		isRayHit_ = false;
		return;
	}

	Vector3 position = triviaObj_->GetTranslate();
	if (!isLanded_) {
		velocity_.y -= 0.02f;
		position += velocity_;
		if (position.y <= 0.5f) {
			position.y = 0.5f;
			velocity_ = {0.0f, 0.0f, 0.0f};
			isLanded_ = true;
		}
	}

	triviaObj_->SetTranslate(position);
	triviaObj_->UpdateBillboard();

	isRayHit_ = false;
	if (isLanded_ && playerCamera_) {
		isRayHit_ = playerCamera_->OnCollisionRay(localAABB_, position);
	}
	if (isRayHit_) {
		triviaObj_->UpdateBillboard();
	}

	if (isRayHit_ && PlayerCommand::GetInstance()->InteractTrigger()) {
		isSendStartTriviaMessage_ = true;

		std::string filePath = "Resources/TD3_3102/Audio/Voice/Coffee/" + std::to_string(triviaNum_) + ".mp3";
		Audio::GetInstance()->SoundUnload(&triviaVoice_);

		if (triviaNum_ < strings_.size() - 1) {
			triviaNum_++;
		} else {
			triviaNum_ = 0;
		}

		filePath = "Resources/TD3_3102/Audio/Voice/Coffee/" + std::to_string(triviaNum_) + ".mp3";
		triviaVoice_ = Audio::GetInstance()->SoundLoadFile(filePath.c_str());
		Audio::GetInstance()->SoundPlayWave(triviaVoice_, false);
	}
}

void CoffeeTrivia::Draw() {
	if (!isActive_) {
		return;
	}
	Object3dCommon::GetInstance()->DrawCommon();
	triviaObj_->Draw();
	if (isRayHit_) {
		Object3dCommon::GetInstance()->DrawCommonOutline();
		triviaObj_->Draw();
		Object3dCommon::GetInstance()->EndOutlineDraw();
	}
}

void CoffeeTrivia::SetVol(float vol) { Audio::GetInstance()->SetSoundVolume(&triviaVoice_, vol); }

void CoffeeTrivia::SetCamera(Camera* camera) {
	if (!triviaObj_) {
		return;
	}
	triviaObj_->SetCamera(camera);
	triviaObj_->UpdateCameraMatrices();
}

void CoffeeTrivia::Spawn(const Vector3& origin, const Vector3& forward) {
	if (!triviaObj_) {
		return;
	}
	spawnForward_ = forward;
	triviaObj_->SetTranslate(origin);
	velocity_ = {forward.x * 0.06f, 0.12f, forward.z * 0.06f};
	isActive_ = true;
	isLanded_ = false;
	isRayHit_ = false;
}