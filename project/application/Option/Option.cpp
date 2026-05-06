#define NOMINMAX
#include "Option.h"
#include "Engine/Loadfile/JSON/JsonManager.h"
#include "Logger.h"
#include "Sprite/SpriteCommon.h"
#include "TextureManager.h"
#include "WinApp.h"
#include "application/GameObject/KeyBindConfig.h"
#include "application/Color/Color.h"
#include "application/GameObject/SEManager/SEManager.h"
#include <algorithm>
namespace {
    const char* kOptionFileName = "optionData.json";

    float Clamp01(float value) { return std::clamp(value, 0.0f, 1.0f); }

    float CameraSensitivityFromDivision(int division) {
        constexpr float kMinCameraSensitivity = 0.1f;
        constexpr float kMaxCameraSensitivity = 2.0f;
        constexpr float kStep = (kMaxCameraSensitivity - kMinCameraSensitivity) / 9.0f;
        return kMinCameraSensitivity + (kStep * static_cast<float>(division));
    }

    int DivisionFromCameraSensitivity(float value) {
        constexpr float kMinCameraSensitivity = 0.1f;
        constexpr float kMaxCameraSensitivity = 2.0f;
        constexpr float kStep = (kMaxCameraSensitivity - kMinCameraSensitivity) / 9.0f;
        const float clamped = std::clamp(value, kMinCameraSensitivity, kMaxCameraSensitivity);
        const int division = static_cast<int>((clamped - kMinCameraSensitivity) / kStep + 0.5f);
        return std::clamp(division, 0, 9);
    }

    int DivisionFromVolume(float value) {
        const float clamped = Clamp01(value);
        const int division = static_cast<int>(clamped * 9.0f + 0.5f);
        return std::clamp(division, 0, 9);
    }

    float FieldOfViewFromDivision(int division) {
        constexpr float kMinFOV = 45.0f;
        constexpr float kMaxFOV = 60.0f;
        constexpr float kStep = (kMaxFOV - kMinFOV) / 9.0f;
        return kMinFOV + (kStep * static_cast<float>(division));
    }

    int DivisionFromFieldOfView(float value) {
        constexpr float kMinFOV = 45.0f;
        constexpr float kMaxFOV = 60.0f;
        constexpr float kStep = (kMaxFOV - kMinFOV) / 9.0f;
        const float clamped = std::clamp(value, kMinFOV, kMaxFOV);
        const int division = static_cast<int>((clamped - kMinFOV) / kStep + 0.5f);
        return std::clamp(division, 0, 9);
    }

} // namespace
OptionData Option::currentOptionData_{};
void Option::Initialize() {
    uint32_t textureHandle = TextureManager::GetInstance()->GetTextureIndexByfilePath("Resources/TD3_3102/2d/white2x2.png");
    fontHandle_ = FreeTypeManager::CreateFace("Resources/TD3_3102/Irohakaku/irohakakuC-Medium.ttf", 0);
    FreeTypeManager::SetPixelSizes(fontHandle_, 42, 42);

    optionTitleText_.Initialize(fontHandle_);
    optionTitleText_.SetString(U"オプション");
    optionTitleText_.SetPosition({ WinApp::kClientWidth / 2.0f, WinApp::kClientHeight / 2.0f - 200.0f });
    optionTitleText_.SetColor(COLOR::WHITE);
    optionTitleText_.SetAlign(TextAlign::Center);
    optionTitleText_.UpdateLayout(false);

    saveHintText_.Initialize(fontHandle_);
    saveHintText_.SetString(U"Eで保存して戻る");
    saveHintText_.SetPosition({ WinApp::kClientWidth / 2.0f, WinApp::kClientHeight - 30.0f });
    saveHintText_.SetColor(COLOR::WHITE);
    saveHintText_.SetAlign(TextAlign::Center);
    saveHintText_.UpdateLayout(false);

    for (int parameterIndex = 0; parameterIndex < kOptionParameterNum; ++parameterIndex) {
        Text& parameterLabel = optionParameterTexts_[parameterIndex];
        parameterLabel.Initialize(fontHandle_);
        parameterLabel.SetString(kParameterLabels_[parameterIndex]);
        parameterLabel.SetPosition({ kOptionLabelPos.x, kOptionLabelPos.y + (kOptionLabelStepY * static_cast<float>(parameterIndex)) });
        parameterLabel.SetColor(COLOR::WHITE);
        parameterLabel.SetAlign(TextAlign::Left);
        parameterLabel.UpdateLayout(false);
    }

    LoadOptionData();

    for (int parameterIndex = 0; parameterIndex < kOptionParameterNum; ++parameterIndex) {
        for (int divisionIndex = 0; divisionIndex < kOptionParameterDivisionNum; ++divisionIndex) {
            Sprite& sprite = parameterSprite_[parameterIndex][divisionIndex];
            sprite.Initialize(textureHandle);
            sprite.SetScale(kOptionSpriteSize);
            sprite.SetPosition(
                { kOptionSpriteStartPos.x + (kOptionSpriteStep.x * static_cast<float>(divisionIndex)), kOptionSpriteStartPos.y + (kOptionSpriteStep.y * static_cast<float>(parameterIndex)) });
        }
    }
}
void Option::Update() {
    if (!isShowOption_) {
        return;
    }

    PlayerCommand* playerCommand = PlayerCommand::GetInstance();
    const bool moveUp = playerCommand->UiMoveForwardTrigger() || playerCommand->MouseWheelDown();
    const bool moveDown = playerCommand->UiMoveBackwardTrigger() || playerCommand->MouseWheelUp();
    const bool moveLeft = playerCommand->UiMoveLeftTrigger();
    const bool moveRight = playerCommand->UiMoveRightTrigger();
    const bool closeOption = playerCommand->UiInteractTrigger() || playerCommand->Shot();

    if (closeOption) {
        SaveOptionData();
        CloseOption();
        SEManager::SoundPlay(SEManager::PUSH_WATCH);
        return;
    }

    if (moveUp) {
        selectedParameterIndex_ = (selectedParameterIndex_ + kOptionParameterNum - 1) % kOptionParameterNum;
        SEManager::SoundPlay(SEManager::PUSH_WATCH);
    } else if (moveDown) {
        selectedParameterIndex_ = (selectedParameterIndex_ + 1) % kOptionParameterNum;
        SEManager::SoundPlay(SEManager::PUSH_WATCH);
    }

    int currentDivision = 0;
    if (selectedParameterIndex_ == 0) {
        //カメラの感度
        currentDivision = DivisionFromCameraSensitivity(optionData_.CameraMoveSpeed.x);
    } else if (selectedParameterIndex_ == 1) {
        // 追加: 視野角
        currentDivision = DivisionFromFieldOfView(optionData_.fieldOfView);
    } else if (selectedParameterIndex_ == 2) {
        //左右反転 (ON=1, OFF=0)	
        currentDivision = optionData_.isFlipHorizontally ? 1 : 0;
    } else if (selectedParameterIndex_ == 3) {
        //BGMVolume
        currentDivision = DivisionFromVolume(optionData_.BGMVolume);
    } else if (selectedParameterIndex_ == 4) {
        //SEVolume
        currentDivision = DivisionFromVolume(optionData_.SEVolume);
    } else if (selectedParameterIndex_ == 5) {
        //VoiceVolume
        currentDivision = DivisionFromVolume(optionData_.VoiceVolume);
    }

    if (moveLeft) {
        const int nextDivision = std::max(0, currentDivision - 1);
        if (nextDivision != currentDivision) {
            SEManager::SoundPlay(SEManager::PUSH_WATCH);
        }
        currentDivision = nextDivision;
    } else if (moveRight) {
        //左右反転の場合は0か1のみ
        int maxDiv = (selectedParameterIndex_ == 2) ? 1 : (kOptionParameterDivisionNum - 1);
        const int nextDivision = std::min(maxDiv, currentDivision + 1);
        if (nextDivision != currentDivision) {
            SEManager::SoundPlay(SEManager::PUSH_WATCH);
        }
        currentDivision = nextDivision;
    }

    if (selectedParameterIndex_ == 0) {
        const float sensitivity = CameraSensitivityFromDivision(currentDivision);
        optionData_.CameraMoveSpeed.x = sensitivity;
        optionData_.CameraMoveSpeed.y = sensitivity;
    } else if (selectedParameterIndex_ == 1) {
        // 追加: 視野角の適用
        optionData_.fieldOfView = FieldOfViewFromDivision(currentDivision);
    } else if (selectedParameterIndex_ == 2) {
        // 追加: 左右反転の適用
        optionData_.isFlipHorizontally = (currentDivision == 1);
    } else if (selectedParameterIndex_ == 3) {
        //BGMVolume
        optionData_.BGMVolume = static_cast<float>(currentDivision) / 9.0f;
    } else if (selectedParameterIndex_ == 4) {
        //SEVolume
        optionData_.SEVolume = static_cast<float>(currentDivision) / 9.0f;
    } else if (selectedParameterIndex_ == 5) {
        //VoiceVolume
        optionData_.VoiceVolume = static_cast<float>(currentDivision) / 9.0f;
    }

    const int cameraDivision = DivisionFromCameraSensitivity(optionData_.CameraMoveSpeed.x);
    const int fovDivision = DivisionFromFieldOfView(optionData_.fieldOfView); // 追加
    const bool flipDivision = optionData_.isFlipHorizontally ? 1 : 0; // 追加


    const int bgmDivision = DivisionFromVolume(optionData_.BGMVolume);
    const int seDivision = DivisionFromVolume(optionData_.SEVolume);
    const int voiceDivision = DivisionFromVolume(optionData_.VoiceVolume);

    const int divisions[kOptionParameterNum] = { cameraDivision,fovDivision, flipDivision,bgmDivision, seDivision, voiceDivision };

    for (int parameterIndex = 0; parameterIndex < kOptionParameterNum; ++parameterIndex) {

        if (parameterIndex >= 3 && divisions[parameterIndex] == 0) {
            optionParameterTexts_[parameterIndex].SetString(kParameterLabels_[parameterIndex] + U"×");
        } else {
            optionParameterTexts_[parameterIndex].SetString(kParameterLabels_[parameterIndex]);
        }
        optionParameterTexts_[parameterIndex].SetColor(parameterIndex == selectedParameterIndex_ ? COLOR::RED : COLOR::WHITE);
        optionParameterTexts_[parameterIndex].UpdateLayout(false);
    }

    for (int parameterIndex = 0; parameterIndex < kOptionParameterNum; ++parameterIndex) {
        for (int divisionIndex = 0; divisionIndex < kOptionParameterDivisionNum; ++divisionIndex) {

            bool isGaugeActive = false;
            if (parameterIndex < 1) {
                isGaugeActive = (divisionIndex <= divisions[parameterIndex]);
            } else {
                isGaugeActive = (divisionIndex < divisions[parameterIndex]);
            }

            if (isGaugeActive) {
                parameterSprite_[parameterIndex][divisionIndex].SetColor(parameterIndex == selectedParameterIndex_ ? COLOR::RED : COLOR::WHITE);
            } else {
                parameterSprite_[parameterIndex][divisionIndex].SetColor({ 0.3f, 0.3f, 0.3f, 1.0f });
            }

            parameterSprite_[parameterIndex][divisionIndex].Update();
        }
    }
}
void Option::Draw() {

    SpriteCommon::GetInstance()->DrawCommonFont();
    optionTitleText_.Draw();
    saveHintText_.Draw();
    for (int parameterIndex = 0; parameterIndex < kOptionParameterNum; ++parameterIndex) {
        optionParameterTexts_[parameterIndex].Draw();
    }

    SpriteCommon::GetInstance()->DrawCommon();
    for (int parameterIndex = 0; parameterIndex < kOptionParameterNum; ++parameterIndex) {

        if (parameterIndex == 2) {
            parameterSprite_[parameterIndex][0].Draw();

        } else {
            for (int divisionIndex = 0; divisionIndex < kOptionParameterDivisionNum; ++divisionIndex) {

                parameterSprite_[parameterIndex][divisionIndex].Draw();
            }
        }

    }
}

void Option::SaveOptionData() {
    nlohmann::json optionJson;
    optionJson["CameraMoveSpeed"] = {
        {"x", optionData_.CameraMoveSpeed.x},
        {"y", optionData_.CameraMoveSpeed.y},
    };

    optionJson["fieldOfView"] = optionData_.fieldOfView; // ここを追加
    optionJson["isFlipHorizontally"] = optionData_.isFlipHorizontally;
    optionJson["isFlipVertically"] = optionData_.isFlipVertically;

    optionJson["BGMVolume"] = optionData_.BGMVolume;
    optionJson["SEVolume"] = optionData_.SEVolume;
    optionJson["VoiceVolume"] = optionData_.VoiceVolume;

    JsonManager* jsonManager = JsonManager::GetInstance();
    jsonManager->SetData(optionJson);
    if (!jsonManager->SaveJson(kOptionFileName)) {
        Logger::Log("オプション設定の保存に失敗しました。\n");
    }
    currentOptionData_ = optionData_;
}

void Option::LoadOptionData() {
    JsonManager* jsonManager = JsonManager::GetInstance();
    if (!jsonManager->LoadJson(kOptionFileName)) {
        SaveOptionData();
        currentOptionData_ = optionData_;
        return;
    }

    const nlohmann::json& optionJson = jsonManager->GetData();

    if (optionJson.contains("CameraMoveSpeed") && optionJson["CameraMoveSpeed"].is_object()) {
        const nlohmann::json& cameraMoveSpeed = optionJson["CameraMoveSpeed"];
        if (cameraMoveSpeed.contains("x") && cameraMoveSpeed["x"].is_number()) {
            optionData_.CameraMoveSpeed.x = cameraMoveSpeed["x"].get<float>();
        }
        if (cameraMoveSpeed.contains("y") && cameraMoveSpeed["y"].is_number()) {
            optionData_.CameraMoveSpeed.y = cameraMoveSpeed["y"].get<float>();
        }
    }

    // ここを追加
    if (optionJson.contains("fieldOfView") && optionJson["fieldOfView"].is_number()) {
        optionData_.fieldOfView = optionJson["fieldOfView"].get<float>();
    }

    if (optionJson.contains("isFlipHorizontally") && optionJson["isFlipHorizontally"].is_number()) {
        optionData_.isFlipHorizontally = optionJson["isFlipHorizontally"].get<float>();
    }
    if (optionJson.contains("isFlipVertically") && optionJson["isFlipVertically"].is_number()) {
        optionData_.isFlipVertically = optionJson["isFlipVertically"].get<float>();
    }

    if (optionJson.contains("BGMVolume") && optionJson["BGMVolume"].is_number()) {
        optionData_.BGMVolume = optionJson["BGMVolume"].get<float>();
    }
    if (optionJson.contains("SEVolume") && optionJson["SEVolume"].is_number()) {
        optionData_.SEVolume = optionJson["SEVolume"].get<float>();
    }
    if (optionJson.contains("VoiceVolume") && optionJson["VoiceVolume"].is_number()) {
        optionData_.VoiceVolume = optionJson["VoiceVolume"].get<float>();
    }

    currentOptionData_ = optionData_;
}