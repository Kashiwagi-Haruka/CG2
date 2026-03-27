#include "Option.h"
#include "Engine/Loadfile/JSON/JsonManager.h"
#include "Logger.h"
#include "Sprite/SpriteCommon.h"
#include "TextureManager.h"
#include "WinApp.h"
#include "application/Color/Color.h"
namespace {
const char* kOptionFileName = "optionData.json";
}

void Option::Initialize() {
	uint32_t textureHandle = TextureManager::GetInstance()->GetTextureIndexByfilePath("Resources/TD3_3102/2d/white2x2.png");
	fontHandle_ = FreeTypeManager::CreateFace("Resources/TD3_3102/Irohakaku/irohakakuC-Medium.ttf", 0);
	FreeTypeManager::SetPixelSizes(fontHandle_, 42, 42);

	optionTitleText_.Initialize(fontHandle_);
	optionTitleText_.SetString(U"オプション");
	optionTitleText_.SetPosition({WinApp::kClientWidth / 2.0f, WinApp::kClientHeight / 2.0f - 180.0f});
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

	for (int parameterIndex = 0; parameterIndex < kOptionParameterNum; ++parameterIndex) {
		for (int divisionIndex = 0; divisionIndex < kOptionParameterDivisionNum; ++divisionIndex) {
			parameterSprite_[parameterIndex][divisionIndex].Update();
		}
	}
}
void Option::Draw() {
	if (!isShowOption_) {
		return;
	}
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