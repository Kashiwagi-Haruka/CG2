#define NOMINMAX
#include "Hinstance.h"
#include "Engine/BaseScene/SceneManager.h"
#include "Engine/Loadfile/JSON/JsonManager.h"
#include "Object3d/Object3d.h"
#ifdef USE_IMGUI
#include "externals/imgui/imgui.h"
#endif

#include <algorithm>
#include <array>
#include <cstring>
#include <filesystem>
#include <string>

namespace {
std::filesystem::path ResolveObjectEditorJsonPath(const std::string& filePath) { return std::filesystem::path("Resources") / "JSON" / std::filesystem::path(filePath).filename(); }

bool HasObjectEditorJsonFile(const std::string& filePath) { return std::filesystem::exists(ResolveObjectEditorJsonPath(filePath)); }
} // namespace

Hinstance* Hinstance::GetInstance() {
	static Hinstance instance;
	return &instance;
}

void Hinstance::RegisterObject3d(Object3d* object) {
	if (!object) {
		return;
	}
	if (std::find(objects_.begin(), objects_.end(), object) == objects_.end()) {
		const size_t index = objects_.size();
		objects_.push_back(object);
		objectNames_.push_back("Object " + std::to_string(index));
		editorTransforms_.push_back(object->GetTransform());
		editorMaterials_.push_back({
		    object->GetColor(),
		    object->IsLightingEnabled(),
		    object->GetShininess(),
		    object->GetEnvironmentCoefficient(),
		    object->IsGrayscaleEnabled(),
		    object->IsSepiaEnabled(),
		});

	}
}
bool Hinstance::LoadObjectEditorsFromJsonIfExists(const std::string& filePath) {
	if (!HasObjectEditorJsonFile(filePath)) {
		return false;
	}

	return LoadObjectEditorsFromJson(filePath);
}
void Hinstance::UnregisterObject3d(Object3d* object) {
	if (!object) {
		return;
	}
	for (size_t i = 0; i < objects_.size(); ++i) {
		if (objects_[i] == object) {
			objects_.erase(objects_.begin() + i);
			objectNames_.erase(objectNames_.begin() + i);
			editorTransforms_.erase(editorTransforms_.begin() + i);
			editorMaterials_.erase(editorMaterials_.begin() + i);
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
		objectJson["name"] = objectNames_[i];
		objectJson["transform"] = {
		    {"scale",     {transform.scale.x, transform.scale.y, transform.scale.z}            },
		    {"rotate",    {transform.rotate.x, transform.rotate.y, transform.rotate.z}         },
		    {"translate", {transform.translate.x, transform.translate.y, transform.translate.z}},
		};
		const EditorMaterial& material = editorMaterials_[i];
		objectJson["material"] = {
		    {"color",                  {material.color.x, material.color.y, material.color.z, material.color.w}},
		    {"enableLighting",         material.enableLighting                                                 },
		    {"shininess",              material.shininess		                                              },
		    {"environmentCoefficient", material.environmentCoefficient                                         },
		    {"grayscaleEnabled",       material.grayscaleEnabled                                               },
		    {"sepiaEnabled",           material.sepiaEnabled		                                           },
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
		if (objectJson.contains("name") && objectJson["name"].is_string()) {
			objectNames_[index] = objectJson["name"].get<std::string>();
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
		EditorMaterial material = editorMaterials_[index];
		if (objectJson.contains("material") && objectJson["material"].is_object()) {
			const auto& materialJson = objectJson["material"];
			if (materialJson.contains("color") && materialJson["color"].is_array() && materialJson["color"].size() == 4) {
				material.color = {
				    materialJson["color"][0].get<float>(),
				    materialJson["color"][1].get<float>(),
				    materialJson["color"][2].get<float>(),
				    materialJson["color"][3].get<float>(),
				};
			}
			if (materialJson.contains("enableLighting") && materialJson["enableLighting"].is_boolean()) {
				material.enableLighting = materialJson["enableLighting"].get<bool>();
			}
			if (materialJson.contains("shininess") && materialJson["shininess"].is_number()) {
				material.shininess = materialJson["shininess"].get<float>();
			}
			if (materialJson.contains("environmentCoefficient") && materialJson["environmentCoefficient"].is_number()) {
				material.environmentCoefficient = materialJson["environmentCoefficient"].get<float>();
			}
			if (materialJson.contains("grayscaleEnabled") && materialJson["grayscaleEnabled"].is_boolean()) {
				material.grayscaleEnabled = materialJson["grayscaleEnabled"].get<bool>();
			}
			if (materialJson.contains("sepiaEnabled") && materialJson["sepiaEnabled"].is_boolean()) {
				material.sepiaEnabled = materialJson["sepiaEnabled"].get<bool>();
			}
		}
		editorMaterials_[index] = material;
		objects_[index]->SetColor(material.color);
		objects_[index]->SetEnableLighting(material.enableLighting);
		objects_[index]->SetShininess(material.shininess);
		objects_[index]->SetEnvironmentCoefficient(material.environmentCoefficient);
		objects_[index]->SetGrayscaleEnabled(material.grayscaleEnabled);
		objects_[index]->SetSepiaEnabled(material.sepiaEnabled);
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
		if (saved) {
			hasUnsavedChanges_ = false;
		}
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
			if (loaded) {
				hasUnsavedChanges_ = false;
			}
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
		std::string& objectName = objectNames_[i];
		std::string nodeLabel = objectName.empty() ? ("Object " + std::to_string(i)) : objectName;
		const std::string treeNodeLabel = nodeLabel + "###node_" + std::to_string(i);
		if (ImGui::TreeNode(treeNodeLabel.c_str())) {
			Transform& transform = editorTransforms_[i];
			EditorMaterial& material = editorMaterials_[i];
			bool transformChanged = false;
			bool materialChanged = false;
			bool nameChanged = false;
			if (isPlaying_) {
				ImGui::TextUnformatted("Playing... editor values are locked");
			} else {
				std::array<char, 128> nameBuffer{};
				const size_t copyLength = std::min(nameBuffer.size() - 1, objectName.size());
				std::copy_n(objectName.begin(), copyLength, nameBuffer.begin());
				if (ImGui::InputText(("Name##" + std::to_string(i)).c_str(), nameBuffer.data(), nameBuffer.size())) {
					objectName = nameBuffer.data();
					nameChanged = true;
				}
				transformChanged |= ImGui::DragFloat3(("Scale##" + std::to_string(i)).c_str(), &transform.scale.x, 0.01f);
				transformChanged |= ImGui::DragFloat3(("Rotate##" + std::to_string(i)).c_str(), &transform.rotate.x, 0.01f);
				transformChanged |= ImGui::DragFloat3(("Translate##" + std::to_string(i)).c_str(), &transform.translate.x, 0.01f);
				ImGui::SeparatorText("Material");
				materialChanged |= ImGui::ColorEdit4(("Color##" + std::to_string(i)).c_str(), &material.color.x);
				materialChanged |= ImGui::Checkbox(("Enable Lighting##" + std::to_string(i)).c_str(), &material.enableLighting);
				materialChanged |= ImGui::DragFloat(("Shininess##" + std::to_string(i)).c_str(), &material.shininess, 0.1f, 0.0f, 256.0f);
				materialChanged |= ImGui::DragFloat(("Environment##" + std::to_string(i)).c_str(), &material.environmentCoefficient, 0.01f, 0.0f, 1.0f);
				materialChanged |= ImGui::Checkbox(("Grayscale##" + std::to_string(i)).c_str(), &material.grayscaleEnabled);
				materialChanged |= ImGui::Checkbox(("Sepia##" + std::to_string(i)).c_str(), &material.sepiaEnabled);
			}
			if (transformChanged || materialChanged || nameChanged) {
				hasUnsavedChanges_ = true;
			}
			if (transformChanged) {
				object->SetTransform(transform);
			}
			if (materialChanged) {
				object->SetColor(material.color);
				object->SetEnableLighting(material.enableLighting);
				object->SetShininess(material.shininess);
				object->SetEnvironmentCoefficient(material.environmentCoefficient);
				object->SetGrayscaleEnabled(material.grayscaleEnabled);
				object->SetSepiaEnabled(material.sepiaEnabled);
			}
			ImGui::TreePop();
		}
	}

	ImGui::End();
#endif
}