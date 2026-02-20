#define NOMINMAX
#include "Hinstance.h"
#include "Engine/BaseScene/SceneManager.h"
#include "Engine/Loadfile/JSON/JsonManager.h"
#include "Object3d/Object3d.h"
#ifdef USE_IMGUI
#include "externals/imgui/imgui.h"
#endif

#include <algorithm>
#include <string>

Hinstance* Hinstance::GetInstance() {
	static Hinstance instance;
	return &instance;
}

void Hinstance::RegisterObject3d(Object3d* object) {
	if (!object) {
		return;
	}
	if (std::find(objects_.begin(), objects_.end(), object) == objects_.end()) {
		objects_.push_back(object);
		editorTransforms_.push_back(object->GetTransform());
	}
}

void Hinstance::UnregisterObject3d(Object3d* object) {
	if (!object) {
		return;
	}
	for (size_t i = 0; i < objects_.size(); ++i) {
		if (objects_[i] == object) {
			objects_.erase(objects_.begin() + i);
			editorTransforms_.erase(editorTransforms_.begin() + i);
			break;
		}
	}
}

bool Hinstance::HasRegisteredObjects() const { return !objects_.empty(); }

bool Hinstance::SaveObjectEditorsToJson(const std::string& filePath) const {
	nlohmann::json root;
	root["objects"] = nlohmann::json::array();

	for (size_t i = 0; i < objects_.size(); ++i) {
		const Object3d* object = objects_[i];
		if (!object) {
			continue;
		}
		const Transform& transform = editorTransforms_[i];
		nlohmann::json objectJson;
		objectJson["index"] = i;
		objectJson["transform"] = {
		    {"scale",     {transform.scale.x, transform.scale.y, transform.scale.z}            },
		    {"rotate",    {transform.rotate.x, transform.rotate.y, transform.rotate.z}         },
		    {"translate", {transform.translate.x, transform.translate.y, transform.translate.z}},
		};
		root["objects"].push_back(objectJson);
	}

	JsonManager* jsonManager = JsonManager::GetInstance();
	jsonManager->SetData(root);
	return jsonManager->SaveJson(filePath);
}

bool Hinstance::LoadObjectEditorsFromJson(const std::string& filePath) {
	JsonManager* jsonManager = JsonManager::GetInstance();
	if (!jsonManager->LoadJson(filePath)) {
		return false;
	}

	const nlohmann::json& root = jsonManager->GetData();
	if (!root.contains("objects") || !root["objects"].is_array()) {
		return false;
	}

	for (const auto& objectJson : root["objects"]) {
		if (!objectJson.contains("index") || !objectJson["index"].is_number_unsigned()) {
			continue;
		}
		const size_t index = objectJson["index"].get<size_t>();
		if (index >= objects_.size() || !objects_[index]) {
			continue;
		}
		if (!objectJson.contains("transform") || !objectJson["transform"].is_object()) {
			continue;
		}
		const auto& transformJson = objectJson["transform"];
		if (!transformJson.contains("scale") || !transformJson["scale"].is_array() || transformJson["scale"].size() != 3) {
			continue;
		}
		if (!transformJson.contains("rotate") || !transformJson["rotate"].is_array() || transformJson["rotate"].size() != 3) {
			continue;
		}
		if (!transformJson.contains("translate") || !transformJson["translate"].is_array() || transformJson["translate"].size() != 3) {
			continue;
		}

		Transform transform = objects_[index]->GetTransform();
		transform.scale = {
		    transformJson["scale"][0].get<float>(),
		    transformJson["scale"][1].get<float>(),
		    transformJson["scale"][2].get<float>(),
		};
		transform.rotate = {
		    transformJson["rotate"][0].get<float>(),
		    transformJson["rotate"][1].get<float>(),
		    transformJson["rotate"][2].get<float>(),
		};
		transform.translate = {
		    transformJson["translate"][0].get<float>(),
		    transformJson["translate"][1].get<float>(),
		    transformJson["translate"][2].get<float>(),
		};
		editorTransforms_[index] = transform;
		objects_[index]->SetTransform(transform);
	}

	return true;
}

void Hinstance::SetPlayMode(bool isPlaying) { isPlaying_ = isPlaying; }

void Hinstance::DrawObjectEditors() {
#ifdef USE_IMGUI
	if (objects_.empty()) {
		return;
	}

	constexpr float kGameWidthRatio = 0.68f;
	constexpr float kEditorMinWidth = 280.0f;
	const ImGuiViewport* viewport = ImGui::GetMainViewport();
	const float editorWidthMax = viewport->WorkSize.x * (1.0f - kGameWidthRatio);
	const float editorMinWidth = std::min(kEditorMinWidth, viewport->WorkSize.x);
	const float editorWidth = std::clamp(editorWidthMax, editorMinWidth, viewport->WorkSize.x);
	const float editorPosX = viewport->WorkPos.x + viewport->WorkSize.x - editorWidth;
	const float editorPosY = viewport->WorkPos.y;

	ImGui::SetNextWindowPos(ImVec2(editorPosX, editorPosY), ImGuiCond_Always);
	ImGui::SetNextWindowSize(ImVec2(editorWidth, viewport->WorkSize.y), ImGuiCond_Always);
	if (!ImGui::Begin("Hinstance", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize)) {
		ImGui::End();
		return;
	}

	ImGui::Text("Auto Object Editor");
	ImGui::Separator();

	if (!isPlaying_ && ImGui::Button("Save To JSON")) {
		const bool saved = SaveObjectEditorsToJson("objectEditors.json");
		saveStatusMessage_ = saved ? "Saved: objectEditors.json" : "Save failed: objectEditors.json";
	}
	if (!saveStatusMessage_.empty()) {
		ImGui::Text("%s", saveStatusMessage_.c_str());
	}

if (!isPlaying_) {
		if (ImGui::Button("Play")) {
			if (hasUnsavedChanges_) {
				saveStatusMessage_ = "Warning: unsaved changes. Save To JSON before Play";
			} else {
				SceneManager::GetInstance()->RequestReinitializeCurrentScene();
				SetPlayMode(true);
				saveStatusMessage_ = "Playing";
			}
		}
	} else {
		if (ImGui::Button("Stop")) {
			const bool loaded = LoadObjectEditorsFromJson("objectEditors.json");
			SetPlayMode(false);
			saveStatusMessage_ = loaded ? "Stopped: loaded objectEditors.json" : "Stop failed: objectEditors.json";
		}
	}

	ImGui::Separator();
	for (size_t i = 0; i < objects_.size(); ++i) {
		Object3d* object = objects_[i];
		if (!object) {
			continue;
		}
		std::string nodeLabel = "Object " + std::to_string(i);
		if (ImGui::TreeNode((nodeLabel + "##node").c_str())) {
			Transform& transform = editorTransforms_[i];
			bool changed = false;
			if (isPlaying_) {
				ImGui::TextUnformatted("Playing... editor values are locked");
			} else {
				changed |= ImGui::DragFloat3(("Scale##" + std::to_string(i)).c_str(), &transform.scale.x, 0.01f);
				changed |= ImGui::DragFloat3(("Rotate##" + std::to_string(i)).c_str(), &transform.rotate.x, 0.01f);
				changed |= ImGui::DragFloat3(("Translate##" + std::to_string(i)).c_str(), &transform.translate.x, 0.01f);
			}
			if (changed) {
				hasUnsavedChanges_ = true;
				object->SetTransform(transform);
			}
			ImGui::TreePop();
		}
	}

	ImGui::End();
#endif
}