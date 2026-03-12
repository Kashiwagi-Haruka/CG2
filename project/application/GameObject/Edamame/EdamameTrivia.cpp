#include "EdamameTrivia.h"
#include"GameObject/KeyBindConfig.h"

EdamameTrivia::EdamameTrivia()
{
    //枝豆

    Audio::GetInstance()->SetSoundVolume(&triviaVoice_, 1.0f);

    strings_[0] = "アセトアルデヒド,C2H4O";
    strings_[1] = "この物質はエタノールの酸化中間代謝物として肝臓で生成され、酢酸を経て最終的に水と二酸化炭素に分解されます。";
    strings_[2] = "DNAやたんぱく質と結合しやすい性質を持ち、発癌など種々のアルコール性臓器障害の発生に関与すると考えられています。";
    strings_[3] = "枝豆に含まれるビタミンB1とナイアシンは";
    strings_[4] = "アルコールの代謝に関わるビタミンです。";
    strings_[5] = "ナイアシンはアルコールの分解に関与する酵素のサポートをしているビタミンで";
    strings_[6] = "ビタミンB1、化学名チアミンは、糖質を分解する際に酵素の働きをサポートします。";
    strings_[7] = "またアルコールを多く摂取した際に通常の代謝経路を超えて代謝が必要になると、ビタミンB1が必要になります。";
}

EdamameTrivia::~EdamameTrivia()
{
    Audio::GetInstance()->SoundUnload(&triviaVoice_);
}

void EdamameTrivia::Initialize()
{
    triviaNum_ = strings_.size()-1;
}

void EdamameTrivia::Update()
{

    if (PlayerCommand::GetInstance()->InteractTrigger()) {

        std::string  filePath = "Resources/TD3_3102/Audio/Voice/Edamame/" + std::to_string(triviaNum_) + ".mp3";
        Audio::GetInstance()->SoundUnload(&triviaVoice_);

        if (triviaNum_ < strings_.size()-1) {
            triviaNum_++;
        } else {
            triviaNum_ = 0;
        }

       filePath = "Resources/TD3_3102/Audio/Voice/Edamame/" + std::to_string(triviaNum_) + ".mp3";
        triviaVoice_ = Audio::GetInstance()->SoundLoadFile(filePath.c_str());
        Audio::GetInstance()->SoundPlayWave(triviaVoice_, false);
    }

}

void EdamameTrivia::Draw()
{
}

void EdamameTrivia::SetVol(float vol)
{

    Audio::GetInstance()->SetSoundVolume(&triviaVoice_, vol);
}
