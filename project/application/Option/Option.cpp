#include "Option.h"
#include "Engine/Loadfile/JSON/JsonManager.h"
#include "Logger.h"
#include "TextureManager.h"
#include "Sprite/SpriteCommon.h"
namespace {
const char* kOptionFileName = "optionData.json";
}

void Option::Initialize() { 
	uint32_t textureHandle = TextureManager::GetInstance()->GetTextureIndexByfilePath("Resources/TD3_3102/2d/white2x2.png");

	LoadOptionData(); 

	for (int parameterIndex = 0; parameterIndex < kOptionParameterNum; ++parameterIndex) {
		for (int divisionIndex = 0; divisionIndex < kOptionParameterDivisionNum; ++divisionIndex) {
		Sprite& sprite = parameterSprite_[parameterIndex][divisionIndex];
		sprite.Initialize(textureHandle);
		sprite.SetScale(kOptionSpriteSize);
		sprite.SetPosition({kOptionSpriteStartPos.x + (kOptionSpriteStep.x * static_cast<float>(divisionIndex)), kOptionSpriteStartPos.y + (kOptionSpriteStep.y * static_cast<float>(parameterIndex))});
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