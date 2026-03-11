#include "GameSave.h"

namespace {
void ToJsonTransform(nlohmann::json& outJson, const Transform& transform) {
	outJson = {
	    {"scale",     {transform.scale.x, transform.scale.y, transform.scale.z}            },
	    {"rotate",    {transform.rotate.x, transform.rotate.y, transform.rotate.z}         },
	    {"translate", {transform.translate.x, transform.translate.y, transform.translate.z}},
	};
}

void ApplyVector3(const nlohmann::json& source, Vector3& destination) {
	if (!source.is_array() || source.size() < 3) {
		return;
	}

	destination.x = source[0].get<float>();
	destination.y = source[1].get<float>();
	destination.z = source[2].get<float>();
}

void FromJsonTransform(const nlohmann::json& source, Transform& transform) {
	if (!source.is_object()) {
		return;
	}

	if (source.contains("scale")) {
		ApplyVector3(source["scale"], transform.scale);
	}
	if (source.contains("rotate")) {
		ApplyVector3(source["rotate"], transform.rotate);
	}
	if (source.contains("translate")) {
		ApplyVector3(source["translate"], transform.translate);
	}
}
} // namespace

GameSave::GameSave() { ResetToDefault(); }

void GameSave::ResetToDefault() {
	playerSaveData_.transform.scale = {1.0f, 1.0f, 1.0f};
	playerSaveData_.transform.rotate = {0.0f, 0.0f, 0.0f};
	playerSaveData_.transform.translate = {0.0f, 0.0f, 0.0f};

	cameraSaveData_.transform.scale = {1.0f, 1.0f, 1.0f};
	cameraSaveData_.transform.rotate = {0.0f, 0.0f, 0.0f};
	cameraSaveData_.transform.translate = {0.0f, 3.0f, -10.0f};
	sceneSaveData_.sceneName = "TitleScene";
	sceneSaveData_.isHaveTimeCardWatch = false;
}

void GameSave::Save() {
	nlohmann::json root;

	ToJsonTransform(root["player"]["transform"], playerSaveData_.transform);
	ToJsonTransform(root["camera"]["transform"], cameraSaveData_.transform);
	root["scene"]["sceneName"] = sceneSaveData_.sceneName;
	root["scene"]["isHaveTimeCardWatch"] = sceneSaveData_.isHaveTimeCardWatch;

	JsonManager* jsonManager = JsonManager::GetInstance();
	jsonManager->SetData(root);
	jsonManager->SaveJson(kSaveFileName);
}

void GameSave::Load() {
	ResetToDefault();

	JsonManager* jsonManager = JsonManager::GetInstance();
	if (!jsonManager->LoadJson(kSaveFileName)) {
		Save();
		return;
	}

	const nlohmann::json& root = jsonManager->GetData();

	if (root.contains("player") && root["player"].is_object() && root["player"].contains("transform")) {
		FromJsonTransform(root["player"]["transform"], playerSaveData_.transform);
	}

	if (root.contains("camera") && root["camera"].is_object() && root["camera"].contains("transform")) {
		FromJsonTransform(root["camera"]["transform"], cameraSaveData_.transform);
	}

	if (root.contains("scene") && root["scene"].is_object()) {
		const nlohmann::json& sceneData = root["scene"];
		if (sceneData.contains("sceneName") && sceneData["sceneName"].is_string()) {
			sceneSaveData_.sceneName = sceneData["sceneName"].get<std::string>();
		}
		if (sceneData.contains("isHaveTimeCardWatch") && sceneData["isHaveTimeCardWatch"].is_boolean()) {
			sceneSaveData_.isHaveTimeCardWatch = sceneData["isHaveTimeCardWatch"].get<bool>();
		}
	}
}