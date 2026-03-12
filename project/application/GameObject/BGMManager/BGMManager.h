#pragma once
#include"Audio.h"

class BGMManager
{
private:
    static SoundData bgm1_;
public:
    static void Load();
    static void Initialize();
    static void Update();
    static void SetVol(float vol);
    static void UnLoad();
};

