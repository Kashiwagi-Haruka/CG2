#include "BGMManager.h"
#include<cassert>
#include<algorithm>
#include"GameObject/YoshidaMath/Easing.h"
std::unordered_map<BGMManager::Data, SoundData>BGMManager::BGMs_;
bool BGMManager::isEdamameSound_ = false;
void BGMManager::Load()
{
    //BGM
    BGMs_[BGM1] = Audio::GetInstance()->SoundLoadFile("Resources/TD3_3102/Audio/BGM/bgm1.mp3");
    BGMs_[EDAMAME] = Audio::GetInstance()->SoundLoadFile("Resources/TD3_3102/Audio/BGM/scl_k1_cb.mp3");
}

void BGMManager::Initialize()
{
    SoundPlay(BGM1,true);
    SetVol(0.125f, BGM1);
    SetVol(0.1f, EDAMAME);
    isEdamameSound_ = false;
}

void BGMManager::Update()
{
    if (isEdamameSound_) {
        SetVol(0.0f, BGM1);
    } else {
        SetVol(0.25f, BGM1);
    }
}

void BGMManager::SetVol(float vol, const Data& data)
{
    assert(BGMs_.contains(data));
    vol = std::clamp(vol, 0.0f, 1.0f);
    Audio::GetInstance()->SetSoundVolume(&BGMs_[data], vol);
}

void BGMManager::SoundPlay(const Data& data,const bool loop)
{
    assert(BGMs_.contains(data));
    Audio::GetInstance()->SoundPlayWave(BGMs_[data], loop);
}

void BGMManager::UnLoad()
{
    Audio::GetInstance()->SoundUnload(&BGMs_[BGM1]);
    Audio::GetInstance()->SoundUnload(&BGMs_[EDAMAME]);
}
