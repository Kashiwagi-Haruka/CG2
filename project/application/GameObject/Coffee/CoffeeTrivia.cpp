#include "CoffeeTrivia.h"
#include"GameObject/KeyBindConfig.h"
#include"Option/Option.h"
#include "Model/ModelManager.h"

CoffeeTrivia::CoffeeTrivia()
{

    SetVol(Option::GetCurrentOptionData().VoiceVolume);
    strings_[0] = "カフェインは、脳を刺激して頭痛や眠気を和らげ、腎臓に作用して尿量を増やし、心臓を刺激して心拍数を上げる働きがあります。";
    strings_[1] = "例えばコーヒー1杯、150mLには約90mgのカフェインが含まれています。";
    strings_[2] = "しかし、短時間に5g以上摂取すると重症化し、特に脳と心臓に深刻な影響が出ます。";
    strings_[3] = "脳では吐き気、頭痛、手足の震え、重症になると興奮、錯乱、幻覚や妄想、痙攣発作などが起きたりします。";
    strings_[4] = "心臓では重症になると心拍数が140回/分を超えて血圧が低下し、危険な不整脈から心停止に至ることがあります。";
    strings_[5] = "そのため、カフェインは短時間に大量摂取せず、適量を間隔をあけて摂ることが重要です。";

    ModelManager::GetInstance()->LoadModel("Resources/TD3_3102/3d/Coffee_billboard", "CoffeeBillboard");
}

CoffeeTrivia::~CoffeeTrivia()
{
    Audio::GetInstance()->SoundUnload(&triviaVoice_);
}

void CoffeeTrivia::Initialize()
{
    triviaNum_ = strings_.size() - 1;
	triviaObj_ = std::make_unique<Object3d>();
	triviaObj_->Initialize();
	triviaObj_->SetModel("CoffeeBillboard");
}

void CoffeeTrivia::Update()
{

    if (PlayerCommand::GetInstance()->InteractTrigger()) {

        std::string  filePath = "Resources/TD3_3102/Audio/Voice/Coffee/" + std::to_string(triviaNum_) + ".mp3";
        Audio::GetInstance()->SoundUnload(&triviaVoice_);

        if (triviaNum_ < strings_.size() - 1) {
            triviaNum_++;
        } else {
            triviaNum_ = 0;
        }

        filePath = "Resources/TD3_3102/Audio/Voice/Edamame/" + std::to_string(triviaNum_) + ".mp3";
        triviaVoice_ = Audio::GetInstance()->SoundLoadFile(filePath.c_str());
        Audio::GetInstance()->SoundPlayWave(triviaVoice_, false);
    }

}

void CoffeeTrivia::Draw()
{
}

void CoffeeTrivia::SetVol(float vol)
{
    Audio::GetInstance()->SetSoundVolume(&triviaVoice_, vol);
}
