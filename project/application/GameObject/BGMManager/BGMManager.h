#pragma once
#include"Audio.h"
#include<unordered_map>
class BGMManager
{
public:
    enum Data {
        BGM1,
        EDAMAME,
    };

private:
    static bool isEdamameSound_;
    static std::unordered_map<BGMManager::Data, SoundData> BGMs_;
public:
    static void Load();
    static void Initialize();
    static void Update();
    static void SetVol(float vol,const Data& data);
    static void SoundPlay(const Data& data, const bool loop = false);
    static void UnLoad();
    static bool GetIsEdamameSound() { return isEdamameSound_; };
    static void SetIsEdamameSound(const bool flag) { isEdamameSound_ = flag; }
};

