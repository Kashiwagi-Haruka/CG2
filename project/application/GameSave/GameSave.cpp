#include "GameSave.h"
#include "Engine/Log/Logger.h"

namespace {
    constexpr const char* kGameSaveFileName = "gameSave_";

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

void GameSave::CameraSave(const CameraSaveData& saveData) {
    cameraSaveData_ = saveData;
}

std::string GameSave::GetFileName(const int slotIndex)
{
    std::string fileName = kGameSaveFileName + std::to_string(slotIndex) + ".json";
    return fileName;
}

std::string GameSave::GetScreenShotFileName(const int slotIndex)
{
    std::string screenShotFileName = "Resources/TD3_3102/2d/SaveScreenShot/" + std::to_string(slotIndex) + ".png";
    return screenShotFileName;
}

std::string GameSave::GetCurrentDateTimeString()
{
    // 1. 現在時刻（システムクロック）を取得
    auto now = std::chrono::system_clock::now();

    // 2. time_t型に変換
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);

    // 3. ローカルタイム（構造体）に変換
    std::tm now_tm;
#ifdef _WIN32
    localtime_s(&now_tm, &now_c); // Windows (MSVC) での安全な書き方
#else
    now_tm = *std::localtime(&now_c); // 標準的な書き方
#endif

    // 4. 文字列にフォーマット
    std::stringstream ss;
    // "%Y/%m/%d %H:%M:%S" -> 2024/05/2015:30:45
    ss << std::put_time(&now_tm, "%Y/%m/%d\n%H:%M");

    return ss.str();
}

void GameSave::PlayerSave(const Transform& transform) { playerSaveData_.transform = transform; }

void GameSave::ProgressSave(const ProgressSaveData& progressSaveData) {
    progressSaveData_ = progressSaveData;
}

void GameSave::Save(const int slotIndex) {
    nlohmann::json saveJson;
    saveJson["player"] = {
        {"transform", TransformToJson(playerSaveData_.transform)},
    };
    saveJson["camera"] = {
        {"transform",          TransformToJson(cameraSaveData_.transform)},
    };
    saveJson["progress"] = {
        {"isGameClear",      progressSaveData_.isGameClear     },
        {"currentStageName", progressSaveData_.currentStageName},
        {"isLightHave",      progressSaveData_.isLightHave     },
           {"isKeyHave",      progressSaveData_.isKeyHave     },
    };

    saveJson["saveDateTime"] = {
        {"saveDateTime",     saveDateTime_    },
    };

    JsonManager* jsonManager = JsonManager::GetInstance();
    jsonManager->SetData(saveJson);

    std::string fileName = GetFileName(slotIndex);

    if (!jsonManager->SaveJson(fileName)) {
        Logger::Log("ゲームセーブの保存に失敗しました。\n");
    }
}

void GameSave::Load(const int slotIndex) {
    JsonManager* jsonManager = JsonManager::GetInstance();

    std::string fileName = GetFileName(slotIndex);

    if (!jsonManager->LoadJson(fileName)) {
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
        if (progressJson.contains("isKeyHave") && progressJson["isKeyHave"].is_boolean()) {
            progressSaveData_.isKeyHave = progressJson["isKeyHave"].get<bool>();
        }
    }

    //Jsonファイルからセーブ時間を取得する
    if (saveJson.contains("saveDateTime") && saveJson["saveDateTime"].is_object()) {
        const nlohmann::json& timeJson = saveJson["saveDateTime"];

        if (timeJson.contains("saveDateTime") && timeJson["saveDateTime"].is_boolean()) {
            saveDateTime_ = timeJson["saveDateTime"].get<std::string>();
        }
    }
}

void GameSave::Reset() {
    playerSaveData_ = {};
    cameraSaveData_ = {};
    progressSaveData_ = {};
}