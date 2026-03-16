#include "BGMManager.h"

SoundData BGMManager::bgm1_ ;
void BGMManager::Load()
{
	//BGM
	bgm1_ = Audio::GetInstance()->SoundLoadFile("Resources/TD3_3102/Audio/BGM/bgm1.mp3");


}

void BGMManager::Initialize()
{
	Audio::GetInstance()->SoundPlayWave(bgm1_, true);
	SetVol(0.125f);
}

void BGMManager::Update()
{
}

void BGMManager::SetVol(float vol)
{
	Audio::GetInstance()->SetSoundVolume(&bgm1_, vol);
}

void BGMManager::UnLoad()
{
	Audio::GetInstance()->SoundUnload(&bgm1_);
}
