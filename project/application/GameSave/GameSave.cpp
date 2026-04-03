#include "GameSave.h"
#include "Engine/Log/Logger.h"

namespace {
constexpr const char* kGameSaveFileName = "gameSave.json";

nlohmann::json Vector3ToJson(const Vector3& vector) {
	return {
	    {"x", vector.x},
	    {"y", vector.y},
	    {"z", vector.z},
	};
}

nlohmann::json TransformToJson(const Transform& transform) {
	return {
	    {"scale",     Vector3ToJson(transform.scale)    },
	    {"rotate",    Vector3ToJson(transform.rotate)   },
	    {"translate", Vector3ToJson(transform.translate)},
	};
}

void JsonToVector3(const nlohmann::json& json, Vector3& outVector) {
	if (!json.is_object()) {
		return;
	}

	if (json.contains("x") && json["x"].is_number()) {
		outVector.x = json["x"].get<float>();
	}
	if (json.contains("y") && json["y"].is_number()) {
		outVector.y = json["y"].get<float>();
	}
	if (json.contains("z") && json["z"].is_number()) {
		outVector.z = json["z"].get<float>();
	}
}

void JsonToTransform(const nlohmann::json& json, Transform& outTransform) {
	if (!json.is_object()) {
		return;
	}

	if (json.contains("scale")) {
		JsonToVector3(json["scale"], outTransform.scale);
	}
	if (json.contains("rotate")) {
		JsonToVector3(json["rotate"], outTransform.rotate);
	}
	if (json.contains("translate")) {
		JsonToVector3(json["translate"], outTransform.translate);
	}
}
} // namespace

void GameSave::CameraSave(const Transform& transform, float rotateSpeed, bool isFlipHorizontally, bool isFlipVertically) {
	cameraSaveData_.transform = transform;
	cameraSaveData_.rotateSpeed = rotateSpeed;
	cameraSaveData_.isFlipHorizontally = isFlipHorizontally;
	cameraSaveData_.isFlipVertically = isFlipVertically;
}

void GameSave::PlayerSave(const Transform& transform) { playerSaveData_.transform = transform; }

void GameSave::ProgressSave(bool isGameClear, const std::string& currentStageName, bool isLightHave) {
	progressSaveData_.isGameClear = isGameClear;
	progressSaveData_.currentStageName = currentStageName;
	progressSaveData_.isLightHave = isLightHave;
}

void GameSave::Save() {
	nlohmann::json saveJson;
	saveJson["player"] = {
	    {"transform", TransformToJson(playerSaveData_.transform)},
	};
	saveJson["camera"] = {
	    {"transform",          TransformToJson(cameraSaveData_.transform)},
	    {"rotateSpeed",        cameraSaveData_.rotateSpeed               },
	    {"isFlipHorizontally", cameraSaveData_.isFlipHorizontally        },
	    {"isFlipVertically",   cameraSaveData_.isFlipVertically          },
	};
	saveJson["progress"] = {
	    {"isGameClear",      progressSaveData_.isGameClear     },
	    {"currentStageName", progressSaveData_.currentStageName},
	    {"isLightHave",      progressSaveData_.isLightHave     },
	};

	JsonManager* jsonManager = JsonManager::GetInstance();
	jsonManager->SetData(saveJson);
	if (!jsonManager->SaveJson(kGameSaveFileName)) {
		Logger::Log("ゲームセーブの保存に失敗しました。\n");
	}
}

void GameSave::Load() {
	JsonManager* jsonManager = JsonManager::GetInstance();
	if (!jsonManager->LoadJson(kGameSaveFileName)) {
		Reset();
		Save();
		return;
	}

	const nlohmann::json& saveJson = jsonManager->GetData();

	if (saveJson.contains("player") && saveJson["player"].is_object()) {
		const nlohmann::json& playerJson = saveJson["player"];
		if (playerJson.contains("transform")) {
			JsonToTransform(playerJson["transform"], playerSaveData_.transform);
		}
	}

	if (saveJson.contains("camera") && saveJson["camera"].is_object()) {
		const nlohmann::json& cameraJson = saveJson["camera"];
		if (cameraJson.contains("transform")) {
			JsonToTransform(cameraJson["transform"], cameraSaveData_.transform);
		}
		if (cameraJson.contains("rotateSpeed") && cameraJson["rotateSpeed"].is_number()) {
			cameraSaveData_.rotateSpeed = cameraJson["rotateSpeed"].get<float>();
		}
		if (cameraJson.contains("isFlipHorizontally") && cameraJson["isFlipHorizontally"].is_boolean()) {
			cameraSaveData_.isFlipHorizontally = cameraJson["isFlipHorizontally"].get<bool>();
		}
		if (cameraJson.contains("isFlipVertically") && cameraJson["isFlipVertically"].is_boolean()) {
			cameraSaveData_.isFlipVertically = cameraJson["isFlipVertically"].get<bool>();
		}
	}

	if (saveJson.contains("progress") && saveJson["progress"].is_object()) {
		const nlohmann::json& progressJson = saveJson["progress"];
		if (progressJson.contains("isGameClear") && progressJson["isGameClear"].is_boolean()) {
			progressSaveData_.isGameClear = progressJson["isGameClear"].get<bool>();
		}
		if (progressJson.contains("currentStageName") && progressJson["currentStageName"].is_string()) {
			progressSaveData_.currentStageName = progressJson["currentStageName"].get<std::string>();
		}
		if (progressJson.contains("isLightHave") && progressJson["isLightHave"].is_boolean()) {
			progressSaveData_.isLightHave = progressJson["isLightHave"].get<bool>();
		}
	}
}

void GameSave::Reset() {
	playerSaveData_ = {};
	cameraSaveData_ = {};
	progressSaveData_ = {};
}