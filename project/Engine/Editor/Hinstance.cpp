#define NOMINMAX
#include "Hinstance.h"
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
	}
}

void Hinstance::UnregisterObject3d(Object3d* object) {
	if (!object) {
		return;
	}
	auto it = std::remove(objects_.begin(), objects_.end(), object);
	objects_.erase(it, objects_.end());
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
		Transform transform = object->GetTransform();
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

void Hinstance::DrawObjectEditors() {
#ifdef USE_IMGUI
	if (objects_.empty()) {
		return;
	}

	constexpr float kGameWidthRatio = 0.68f;
	constexpr float kEditorMinWidth = 280.0f;
	const ImGuiViewport* viewport = ImGui::GetMainViewport();
	const float editorWidthMax = viewport->WorkSize.x * (1.0f - kGameWidthRatio);
	const float editorWidth = std::max(kEditorMinWidth, std::min(editorWidthMax, viewport->WorkSize.x));
	const float editorPosX = viewport->WorkPos.x + viewport->WorkSize.x - editorWidth;
	const float editorPosY = viewport->WorkPos.y;

	ImGui::SetNextWindowPos(ImVec2(editorPosX, editorPosY), ImGuiCond_Always);

	if (!ImGui::Begin("Hinstance", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize)) {
		ImGui::End();
		return;
	}

	ImGui::Text("Auto Object Editor");
	ImGui::Separator();

	if (ImGui::Button("Save To JSON")) {
		const bool saved = SaveObjectEditorsToJson("objectEditors.json");
		saveStatusMessage_ = saved ? "Saved: objectEditors.json" : "Save failed: objectEditors.json";
	}
	if (!saveStatusMessage_.empty()) {
		ImGui::Text("%s", saveStatusMessage_.c_str());
	}
	ImGui::Separator();
	for (size_t i = 0; i < objects_.size(); ++i) {
		Object3d* object = objects_[i];
		if (!object) {
			continue;
		}
		std::string nodeLabel = "Object " + std::to_string(i);
		if (ImGui::TreeNode((nodeLabel + "##node").c_str())) {
			Transform transform = object->GetTransform();
			bool changed = false;
			changed |= ImGui::DragFloat3(("Scale##" + std::to_string(i)).c_str(), &transform.scale.x, 0.01f);
			changed |= ImGui::DragFloat3(("Rotate##" + std::to_string(i)).c_str(), &transform.rotate.x, 0.01f);
			changed |= ImGui::DragFloat3(("Translate##" + std::to_string(i)).c_str(), &transform.translate.x, 0.01f);
			if (changed) {
				object->SetTransform(transform);
			}
			ImGui::TreePop();
		}
	}

	ImGui::End();
#endif
}