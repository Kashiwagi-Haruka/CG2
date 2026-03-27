#define NOMINMAX
#include "Option.h"
#include "Engine/Loadfile/JSON/JsonManager.h"
#include "Input.h"
#include "Logger.h"
#include "Sprite/SpriteCommon.h"
#include "TextureManager.h"
#include "WinApp.h"
#include "application/Color/Color.h"
#include <algorithm>
namespace {
const char* kOptionFileName = "optionData.json";

float Clamp01(float value) { return std::clamp(value, 0.0f, 1.0f); }

float CameraSensitivityFromDivision(int division) {
	constexpr float kMinCameraSensitivity = 0.5f;
	constexpr float kMaxCameraSensitivity = 2.0f;
	constexpr float kStep = (kMaxCameraSensitivity - kMinCameraSensitivity) / 9.0f;
	return kMinCameraSensitivity + (kStep * static_cast<float>(division));
}

int DivisionFromCameraSensitivity(float value) {
	constexpr float kMinCameraSensitivity = 0.5f;
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
} // namespace

void Option::Initialize() {
	uint32_t textureHandle = TextureManager::GetInstance()->GetTextureIndexByfilePath("Resources/TD3_3102/2d/white2x2.png");
	fontHandle_ = FreeTypeManager::CreateFace("Resources/TD3_3102/Irohakaku/irohakakuC-Medium.ttf", 0);
	FreeTypeManager::SetPixelSizes(fontHandle_, 42, 42);

	optionTitleText_.Initialize(fontHandle_);
	optionTitleText_.SetString(U"オプション");
	optionTitleText_.SetPosition({WinApp::kClientWidth / 2.0f, WinApp::kClientHeight / 2.0f - 200.0f});
	optionTitleText_.SetColor(COLOR::WHITE);
	optionTitleText_.SetAlign(TextAlign::Center);
	optionTitleText_.UpdateLayout(false);

	for (int parameterIndex = 0; parameterIndex < kOptionParameterNum; ++parameterIndex) {
		Text& parameterLabel = optionParameterTexts_[parameterIndex];
		parameterLabel.Initialize(fontHandle_);
		parameterLabel.SetString(kParameterLabels_[parameterIndex]);
		parameterLabel.SetPosition({kOptionLabelPos.x, kOptionLabelPos.y + (kOptionLabelStepY * static_cast<float>(parameterIndex))});
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
			    {kOptionSpriteStartPos.x + (kOptionSpriteStep.x * static_cast<float>(divisionIndex)), kOptionSpriteStartPos.y + (kOptionSpriteStep.y * static_cast<float>(parameterIndex))});
		}
	}
}
void Option::Update() {
	if (!isShowOption_) {
		return;
	}

	auto* input = Input::GetInstance();
	const bool moveUp = input->TriggerKey(DIK_W) || input->TriggerKey(DIK_UP);
	const bool moveDown = input->TriggerKey(DIK_S) || input->TriggerKey(DIK_DOWN);
	const bool moveLeft = input->TriggerKey(DIK_A) || input->TriggerKey(DIK_LEFT);
	const bool moveRight = input->TriggerKey(DIK_D) || input->TriggerKey(DIK_RIGHT);
	const bool closeOption = input->TriggerKey(DIK_ESCAPE) || input->TriggerKey(DIK_Q);

	if (closeOption) {
		SaveOptionData();
		CloseOption();
		return;
	}

	if (moveUp) {
		selectedParameterIndex_ = (selectedParameterIndex_ + kOptionParameterNum - 1) % kOptionParameterNum;
	} else if (moveDown) {
		selectedParameterIndex_ = (selectedParameterIndex_ + 1) % kOptionParameterNum;
	}

	int currentDivision = 0;
	if (selectedParameterIndex_ == 0) {
		currentDivision = DivisionFromCameraSensitivity(optionData_.CameraMoveSpeed.x);
	} else if (selectedParameterIndex_ == 1) {
		currentDivision = DivisionFromVolume(optionData_.BGMVolume);
	} else if (selectedParameterIndex_ == 2) {
		currentDivision = DivisionFromVolume(optionData_.SEVolume);
	} else if (selectedParameterIndex_ == 3) {
		currentDivision = DivisionFromVolume(optionData_.VoiceVolume);
	}

	if (moveLeft) {
		currentDivision = std::max(0, currentDivision - 1);
	} else if (moveRight) {
		currentDivision = std::min(kOptionParameterDivisionNum - 1, currentDivision + 1);
	}

	if (selectedParameterIndex_ == 0) {
		const float sensitivity = CameraSensitivityFromDivision(currentDivision);
		optionData_.CameraMoveSpeed.x = sensitivity;
		optionData_.CameraMoveSpeed.y = sensitivity;
	} else if (selectedParameterIndex_ == 1) {
		optionData_.BGMVolume = static_cast<float>(currentDivision) / 9.0f;
	} else if (selectedParameterIndex_ == 2) {
		optionData_.SEVolume = static_cast<float>(currentDivision) / 9.0f;
	} else if (selectedParameterIndex_ == 3) {
		optionData_.VoiceVolume = static_cast<float>(currentDivision) / 9.0f;
	}

	for (int parameterIndex = 0; parameterIndex < kOptionParameterNum; ++parameterIndex) {
		optionParameterTexts_[parameterIndex].SetColor(parameterIndex == selectedParameterIndex_ ? COLOR::RED : COLOR::WHITE);
		optionParameterTexts_[parameterIndex].UpdateLayout(false);
	}

	const int cameraDivision = DivisionFromCameraSensitivity(optionData_.CameraMoveSpeed.x);
	const int bgmDivision = DivisionFromVolume(optionData_.BGMVolume);
	const int seDivision = DivisionFromVolume(optionData_.SEVolume);
	const int voiceDivision = DivisionFromVolume(optionData_.VoiceVolume);
	const int divisions[kOptionParameterNum] = {cameraDivision, bgmDivision, seDivision, voiceDivision};

	for (int parameterIndex = 0; parameterIndex < kOptionParameterNum; ++parameterIndex) {
		for (int divisionIndex = 0; divisionIndex < kOptionParameterDivisionNum; ++divisionIndex) {
			if (divisionIndex <= divisions[parameterIndex]) {
				parameterSprite_[parameterIndex][divisionIndex].SetColor(parameterIndex == selectedParameterIndex_ ? COLOR::RED : COLOR::WHITE);
			} else {
				parameterSprite_[parameterIndex][divisionIndex].SetColor({0.3f, 0.3f, 0.3f, 1.0f});
			}
			parameterSprite_[parameterIndex][divisionIndex].Update();
		}
	}
}
void Option::Draw() {

	SpriteCommon::GetInstance()->DrawCommonFont();
	optionTitleText_.Draw();
	for (int parameterIndex = 0; parameterIndex < kOptionParameterNum; ++parameterIndex) {
		optionParameterTexts_[parameterIndex].Draw();
	}

	SpriteCommon::GetInstance()->DrawCommon();
	for (int parameterIndex = 0; parameterIndex < kOptionParameterNum; ++parameterIndex) {
		for (int divisionIndex = 0; divisionIndex < kOptionParameterDivisionNum; ++divisionIndex) {
			parameterSprite_[parameterIndex][divisionIndex].Draw();
		}
	}
}

void Option::SaveOptionData() {
	nlohmann::json optionJson;
	optionJson["CameraMoveSpeed"] = {
	    {"x", optionData_.CameraMoveSpeed.x},
	    {"y", optionData_.CameraMoveSpeed.y},
	};
	optionJson["BGMVolume"] = optionData_.BGMVolume;
	optionJson["SEVolume"] = optionData_.SEVolume;
	optionJson["VoiceVolume"] = optionData_.VoiceVolume;

	JsonManager* jsonManager = JsonManager::GetInstance();
	jsonManager->SetData(optionJson);
	if (!jsonManager->SaveJson(kOptionFileName)) {
		Logger::Log("オプション設定の保存に失敗しました。\n");
	}
}
void Option::LoadOptionData() {
	JsonManager* jsonManager = JsonManager::GetInstance();
	if (!jsonManager->LoadJson(kOptionFileName)) {
		SaveOptionData();
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

	if (optionJson.contains("BGMVolume") && optionJson["BGMVolume"].is_number()) {
		optionData_.BGMVolume = optionJson["BGMVolume"].get<float>();
	}
	if (optionJson.contains("SEVolume") && optionJson["SEVolume"].is_number()) {
		optionData_.SEVolume = optionJson["SEVolume"].get<float>();
	}
	if (optionJson.contains("VoiceVolume") && optionJson["VoiceVolume"].is_number()) {
		optionData_.VoiceVolume = optionJson["VoiceVolume"].get<float>();
	}
}