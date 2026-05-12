#pragma once

#include"Audio.h"
#include<array>
#include "Object3d/Object3d.h"
#include <memory>
class CoffeeTrivia
{
private:
    SoundData triviaVoice_;
    std::array<std::string, 6> strings_;
    size_t triviaNum_ = 0;
	std::unique_ptr<Object3d> triviaObj_;

public:
    CoffeeTrivia();
    ~CoffeeTrivia();
    void Initialize();
    void Update();
    void Draw();
    void SetVol(float vol);
};