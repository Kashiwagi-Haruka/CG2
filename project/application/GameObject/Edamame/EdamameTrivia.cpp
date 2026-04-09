#include "EdamameTrivia.h"
#include"GameObject/KeyBindConfig.h"

std::array<std::u32string, 8> EdamameTrivia::strings_;
size_t EdamameTrivia::triviaNum_ = 0;
bool EdamameTrivia::isSendStartTriviaMessage_ = false;
bool EdamameTrivia::isDraw_ = false;
bool EdamameTrivia::isDie_ = false;

EdamameTrivia::EdamameTrivia()
{
    //枝豆

    Audio::GetInstance()->SetSoundVolume(&triviaVoice_, 1.0f);

    strings_[0] = U"アセトアルデヒド,C2H4O";
    strings_[1] = U"この物質はエタノールの酸化中間代謝物として\n肝臓で生成され酢酸を経て\n最終的に水と二酸化炭素に分解されます。";
    strings_[2] = U"DNAやたんぱく質と結合しやすい性質を持ち、\n発癌など種々のアルコール性臓器障害の\n発生に関与すると考えられています。";
    strings_[3] = U"枝豆に含まれるビタミンB1とナイアシンは";
    strings_[4] = U"アルコールの代謝に関わるビタミンです。";
    strings_[5] = U"ナイアシンはアルコールの分解に関与する酵素の\nサポートをしているビタミンで";
    strings_[6] = U"ビタミンB1(化学名チアミン)は、\n糖質を分解する際に酵素の働きをサポートします。";
    strings_[7] = U"またアルコールを多く摂取した際に\n通常の代謝経路を超えて代謝が必要になると、\nビタミンB1が必要になります。";
}

EdamameTrivia::~EdamameTrivia()
{
    Audio::GetInstance()->SoundUnload(&triviaVoice_);
}

void EdamameTrivia::Initialize()
{
    triviaNum_ = strings_.size() - 1;
    count_ = 0;
    isSendStartTriviaMessage_ = false;
    isDraw_ = false;
    isDie_ = false;
    isEnd_ = false;
}

void EdamameTrivia::Update()
{
    isSendStartTriviaMessage_ = false;

    if (isDie_) {
        return;
    }

    if (isEnd_) {
        //サウンドが自然に終了した時とインプットした時で分ける
        if (PlayerCommand::GetInstance()->InteractTrigger()) {
            count_++;
        }

        if (Audio::GetInstance()->IsSoundFinished(triviaVoice_)&&
            triviaNum_ == strings_.size() - 1) {
            count_ = 2;
        }

        if (count_ > 1) {
            Audio::GetInstance()->SoundUnload(&triviaVoice_);
            isDie_ = true;
        }
    } else {

        if (PlayerCommand::GetInstance()->InteractTrigger() || Audio::GetInstance()->IsSoundFinished(triviaVoice_) && triviaNum_ != strings_.size() - 1) {

            SetSound();

            if (triviaNum_ == strings_.size() - 1) {
                isEnd_ = true;
            }
        }
    }

}

void EdamameTrivia::SetSound()
{

    isSendStartTriviaMessage_ = true;
    std::string  filePath = "Resources/TD3_3102/Audio/Voice/Edamame/" + std::to_string(triviaNum_) + ".mp3";
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

void EdamameTrivia::SetVol(float vol)
{

    Audio::GetInstance()->SetSoundVolume(&triviaVoice_, vol);
}
