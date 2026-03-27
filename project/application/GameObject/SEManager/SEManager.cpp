#include "SEManager.h"
#include<cassert>
#include<algorithm>
#include"GameObject/YoshidaMath/Easing.h"
std::unordered_map<SEManager::Data, SoundData>SEManager::SEs_;

void SEManager::Load()
{
    auto* audio = Audio::GetInstance();
    SEs_[DOOR_LOCK] =audio->SoundLoadFile("Resources/TD3_3102/Audio/SE/doorLock.mp3");
    SEs_[DOOR_OPEN] =audio->SoundLoadFile("Resources/TD3_3102/Audio/SE/doorOpen.mp3");
    SEs_[FOOT_STEP] =audio->SoundLoadFile("Resources/TD3_3102/Audio/SE/maou_se_sound_footstep02.mp3");

    SEs_[WARP] = audio->SoundLoadFile("Resources/TD3_3102/Audio/SE/warp0.mp3");
    SEs_[PORTAL_SPAWN] = audio->SoundLoadFile("Resources/TD3_3102/Audio/SE/warp1.mp3");
    SEs_[SHOT] = audio->SoundLoadFile("Resources/TD3_3102/Audio/SE/shot.mp3");
    SEs_[PUSH_WATCH] = audio->SoundLoadFile("Resources/TD3_3102/Audio/SE/pushWatch.mp3");
    SEs_[NOISE] = audio->SoundLoadFile("Resources/TD3_3102/Audio/SE/noise.mp3");
    SEs_[TYPE] = audio->SoundLoadFile("Resources/TD3_3102/Audio/SE/voiceSE.mp3");
}

void SEManager::Initialize()
{
    SetVol(0.25f, DOOR_LOCK);
    SetVol(0.25f, DOOR_OPEN);

    SetVol(0.25f, WARP);
    SetVol(0.25f, PORTAL_SPAWN);
    SetVol(0.25f, SHOT);
    SetVol(1.0f, PUSH_WATCH);
    SetVol(0.0f, NOISE);

    SetVol(1.0f, TYPE);
}

void SEManager::Update()
{

}

void SEManager::SetVol(float vol, const Data& data)
{
    assert(SEs_.contains(data));
    vol = std::clamp(vol, 0.0f, 1.0f);
    Audio::GetInstance()->SetSoundVolume(&SEs_[data], vol);
}

void SEManager::SoundPlay(const Data& data, const bool loop)
{
    assert(SEs_.contains(data));
    Audio::GetInstance()->SoundPlayWave(SEs_[data], loop);
}

void SEManager::UnLoad()
{
    Audio::GetInstance()->SoundUnload(&SEs_[DOOR_LOCK]);
    Audio::GetInstance()->SoundUnload(&SEs_[DOOR_OPEN]);

    Audio::GetInstance()->SoundUnload(&SEs_[WARP]);
    Audio::GetInstance()->SoundUnload(&SEs_[PORTAL_SPAWN]);
    Audio::GetInstance()->SoundUnload(&SEs_[SHOT]);
    Audio::GetInstance()->SoundUnload(&SEs_[PUSH_WATCH]);
    Audio::GetInstance()->SoundUnload(&SEs_[NOISE]);
    Audio::GetInstance()->SoundUnload(&SEs_[TYPE]);
}
