#define NOMINMAX
#include "Hinstance.h"
#include "Engine/BaseScene/SceneManager.h"
#include "Engine/Loadfile/JSON/JsonManager.h"
#include "Function.h"
#include "Object3d/Object3d.h"
#include "Primitive/Primitive.h"
#ifdef USE_IMGUI
#include "externals/imgui/imgui.h"
#endif

#include <algorithm>
#include <array>
#include <cmath>
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
std::string Hinstance::GetSceneScopedEditorFilePath(const std::string& defaultFilePath) const {
	const SceneManager* sceneManager = SceneManager::GetInstance();
	if (!sceneManager) {
		return defaultFilePath;
	}
	const std::string& sceneName = sceneManager->GetCurrentSceneName();
	if (sceneName.empty()) {
		return defaultFilePath;
	}
	return sceneName + "_" + std::filesystem::path(defaultFilePath).filename().string();
}

void Hinstance::ResetForSceneChange() {
	hasUnsavedChanges_ = false;
	saveStatusMessage_.clear();
	hasLoadedForCurrentScene_ = false;
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
		    object->GetDistortionStrength(),
		    object->GetDistortionFalloff(),
		});
	}
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

void Hinstance::RegisterPrimitive(Primitive* primitive) {
	if (!primitive) {
		return;
	}
	if (std::find(primitives_.begin(), primitives_.end(), primitive) == primitives_.end()) {
		const size_t index = primitives_.size();
		primitives_.push_back(primitive);
		primitiveNames_.push_back("Primitive " + std::to_string(index));
		primitiveEditorTransforms_.push_back(primitive->GetTransform());
		primitiveEditorMaterials_.push_back({
		    primitive->GetColor(),
		    primitive->IsLightingEnabled(),
		    primitive->GetShininess(),
		    primitive->GetEnvironmentCoefficient(),
		    primitive->IsGrayscaleEnabled(),
		    primitive->IsSepiaEnabled(),
		    primitive->GetDistortionStrength(),
		    primitive->GetDistortionFalloff(), 
		});
	}
}

void Hinstance::UnregisterPrimitive(Primitive* primitive) {
	if (!primitive) {
		return;
	}
	for (size_t i = 0; i < primitives_.size(); ++i) {
		if (primitives_[i] == primitive) {
			primitives_.erase(primitives_.begin() + i);
			primitiveNames_.erase(primitiveNames_.begin() + i);
			primitiveEditorTransforms_.erase(primitiveEditorTransforms_.begin() + i);
			primitiveEditorMaterials_.erase(primitiveEditorMaterials_.begin() + i);
			break;
		}
	}
}

bool Hinstance::HasRegisteredObjects() const { return !objects_.empty() || !primitives_.empty(); }

bool Hinstance::LoadObjectEditorsFromJsonIfExists(const std::string& filePath) {
	const SceneManager* sceneManager = SceneManager::GetInstance();
	const std::string sceneName = sceneManager ? sceneManager->GetCurrentSceneName() : std::string();
	if (sceneName != loadedSceneName_) {
		ResetForSceneChange();
		loadedSceneName_ = sceneName;
	}

	const std::string scopedFilePath = GetSceneScopedEditorFilePath(filePath);
	if (!HasObjectEditorJsonFile(scopedFilePath)) {
		return false;
	}
	hasLoadedForCurrentScene_ = LoadObjectEditorsFromJson(scopedFilePath);
	return hasLoadedForCurrentScene_;
}

bool Hinstance::SaveObjectEditorsToJson(const std::string& filePath) const {
	nlohmann::json root;
	root["objects"] = nlohmann::json::array();
	root["primitives"] = nlohmann::json::array();

	for (size_t i = 0; i < objects_.size(); ++i) {
		const Object3d* object = objects_[i];
		if (!object) {
			continue;
		}
		const Transform& transform = editorTransforms_[i];
		const EditorMaterial& material = editorMaterials_[i];
		nlohmann::json objectJson;
		objectJson["index"] = i;
		objectJson["name"] = objectNames_[i];
		objectJson["transform"] = {
		    {"scale",     {transform.scale.x, transform.scale.y, transform.scale.z}            },
		    {"rotate",    {transform.rotate.x, transform.rotate.y, transform.rotate.z}         },
		    {"translate", {transform.translate.x, transform.translate.y, transform.translate.z}},
		};
		objectJson["material"] = {
		    {"color",                  {material.color.x, material.color.y, material.color.z, material.color.w}},
		    {"enableLighting",         material.enableLighting                                                 },
		    {"shininess",              material.shininess		                                              },
		    {"environmentCoefficient", material.environmentCoefficient                                         },
		    {"grayscaleEnabled",       material.grayscaleEnabled                                               },
		    {"sepiaEnabled",           material.sepiaEnabled		                                           },
		    {"distortionStrength",     material.distortionStrength                                             },
		    {"distortionFalloff",      material.distortionFalloff                                              },
		    {"uvScale",                {material.uvScale.x, material.uvScale.y, material.uvScale.z}            },
		    {"uvRotate",               {material.uvRotate.x, material.uvRotate.y, material.uvRotate.z}         },
		    {"uvTranslate",            {material.uvTranslate.x, material.uvTranslate.y, material.uvTranslate.z}},
		    {"uvAnchor",               {material.uvAnchor.x, material.uvAnchor.y}                              },
		};
		root["objects"].push_back(objectJson);
	}


	for (size_t i = 0; i < primitives_.size(); ++i) {
		const Primitive* primitive = primitives_[i];
		if (!primitive) {
			continue;
		}
		const Transform& transform = primitiveEditorTransforms_[i];
		const EditorMaterial& material = primitiveEditorMaterials_[i];
		nlohmann::json primitiveJson;
		primitiveJson["index"] = i;
		primitiveJson["name"] = primitiveNames_[i];
		primitiveJson["transform"] = {
		    {"scale",     {transform.scale.x, transform.scale.y, transform.scale.z}            },
		    {"rotate",    {transform.rotate.x, transform.rotate.y, transform.rotate.z}         },
		    {"translate", {transform.translate.x, transform.translate.y, transform.translate.z}},
		};
		primitiveJson["material"] = {
		    {"color",                  {material.color.x, material.color.y, material.color.z, material.color.w}},
		    {"enableLighting",         material.enableLighting                                                 },
		    {"shininess",              material.shininess		                                              },
		    {"environmentCoefficient", material.environmentCoefficient                                         },
		    {"grayscaleEnabled",       material.grayscaleEnabled                                               },
		    {"sepiaEnabled",           material.sepiaEnabled		                                           },
		    {"distortionStrength",     material.distortionStrength                                             },
		    {"distortionFalloff",      material.distortionFalloff                                              },
		    {"uvScale",                {material.uvScale.x, material.uvScale.y, material.uvScale.z}            },
		    {"uvRotate",               {material.uvRotate.x, material.uvRotate.y, material.uvRotate.z}         },
		    {"uvTranslate",            {material.uvTranslate.x, material.uvTranslate.y, material.uvTranslate.z}},
		    {"uvAnchor",               {material.uvAnchor.x, material.uvAnchor.y}                              },
		};
		root["primitives"].push_back(primitiveJson);
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
	if (!root.is_object()) {
		return false;
	}

	if (root.contains("objects") && root["objects"].is_array()) {
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
			if (objectJson.contains("transform") && objectJson["transform"].is_object()) {
				const auto& transformJson = objectJson["transform"];
				if (transformJson.contains("scale") && transformJson["scale"].is_array() && transformJson["scale"].size() == 3 && transformJson.contains("rotate") &&
				    transformJson["rotate"].is_array() && transformJson["rotate"].size() == 3 && transformJson.contains("translate") && transformJson["translate"].is_array() &&
				    transformJson["translate"].size() == 3) {
					Transform transform = objects_[index]->GetTransform();
					transform.scale = {transformJson["scale"][0].get<float>(), transformJson["scale"][1].get<float>(), transformJson["scale"][2].get<float>()};
					transform.rotate = {transformJson["rotate"][0].get<float>(), transformJson["rotate"][1].get<float>(), transformJson["rotate"][2].get<float>()};
					transform.translate = {transformJson["translate"][0].get<float>(), transformJson["translate"][1].get<float>(), transformJson["translate"][2].get<float>()};
					editorTransforms_[index] = transform;
					objects_[index]->SetTransform(transform);
				}
			}
			EditorMaterial material = editorMaterials_[index];
			if (objectJson.contains("material") && objectJson["material"].is_object()) {
				const auto& materialJson = objectJson["material"];
				if (materialJson.contains("color") && materialJson["color"].is_array() && materialJson["color"].size() == 4) {
					material.color = {materialJson["color"][0].get<float>(), materialJson["color"][1].get<float>(), materialJson["color"][2].get<float>(), materialJson["color"][3].get<float>()};
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
				if (materialJson.contains("distortionStrength") && materialJson["distortionStrength"].is_number()) {
					material.distortionStrength = materialJson["distortionStrength"].get<float>();
				}
				if (materialJson.contains("distortionFalloff") && materialJson["distortionFalloff"].is_number()) {
					material.distortionFalloff = materialJson["distortionFalloff"].get<float>();
				}
				if (materialJson.contains("uvScale") && materialJson["uvScale"].is_array() && materialJson["uvScale"].size() == 3) {
					material.uvScale = {materialJson["uvScale"][0].get<float>(), materialJson["uvScale"][1].get<float>(), materialJson["uvScale"][2].get<float>()};
				}
				if (materialJson.contains("uvRotate") && materialJson["uvRotate"].is_array() && materialJson["uvRotate"].size() == 3) {
					material.uvRotate = {materialJson["uvRotate"][0].get<float>(), materialJson["uvRotate"][1].get<float>(), materialJson["uvRotate"][2].get<float>()};
				}
				if (materialJson.contains("uvTranslate") && materialJson["uvTranslate"].is_array() && materialJson["uvTranslate"].size() == 3) {
					material.uvTranslate = {materialJson["uvTranslate"][0].get<float>(), materialJson["uvTranslate"][1].get<float>(), materialJson["uvTranslate"][2].get<float>()};
				}
				if (materialJson.contains("uvAnchor") && materialJson["uvAnchor"].is_array() && materialJson["uvAnchor"].size() == 2) {
					material.uvAnchor = {materialJson["uvAnchor"][0].get<float>(), materialJson["uvAnchor"][1].get<float>()};
				}
			}
			editorMaterials_[index] = material;
			objects_[index]->SetColor(material.color);
			objects_[index]->SetEnableLighting(material.enableLighting);
			objects_[index]->SetShininess(material.shininess);
			objects_[index]->SetEnvironmentCoefficient(material.environmentCoefficient);
			objects_[index]->SetGrayscaleEnabled(material.grayscaleEnabled);
			objects_[index]->SetSepiaEnabled(material.sepiaEnabled);
			objects_[index]->SetDistortionStrength(material.distortionStrength);
			objects_[index]->SetDistortionFalloff(material.distortionFalloff);
			objects_[index]->SetUvTransform(material.uvScale, material.uvRotate, material.uvTranslate, material.uvAnchor);
		}
	}

	if (root.contains("primitives") && root["primitives"].is_array()) {
		for (const auto& primitiveJson : root["primitives"]) {
			if (!primitiveJson.contains("index") || !primitiveJson["index"].is_number_unsigned()) {
				continue;
			}
			const size_t index = primitiveJson["index"].get<size_t>();
			if (index >= primitives_.size() || !primitives_[index]) {
				continue;
			}
			if (primitiveJson.contains("name") && primitiveJson["name"].is_string()) {
				primitiveNames_[index] = primitiveJson["name"].get<std::string>();
			}
			if (primitiveJson.contains("transform") && primitiveJson["transform"].is_object()) {
				const auto& transformJson = primitiveJson["transform"];
				if (transformJson.contains("scale") && transformJson["scale"].is_array() && transformJson["scale"].size() == 3 && transformJson.contains("rotate") &&
				    transformJson["rotate"].is_array() && transformJson["rotate"].size() == 3 && transformJson.contains("translate") && transformJson["translate"].is_array() &&
				    transformJson["translate"].size() == 3) {
					Transform transform = primitives_[index]->GetTransform();
					transform.scale = {transformJson["scale"][0].get<float>(), transformJson["scale"][1].get<float>(), transformJson["scale"][2].get<float>()};
					transform.rotate = {transformJson["rotate"][0].get<float>(), transformJson["rotate"][1].get<float>(), transformJson["rotate"][2].get<float>()};
					transform.translate = {transformJson["translate"][0].get<float>(), transformJson["translate"][1].get<float>(), transformJson["translate"][2].get<float>()};
					primitiveEditorTransforms_[index] = transform;
					primitives_[index]->SetTransform(transform);
				}
			}
			EditorMaterial material = primitiveEditorMaterials_[index];
			if (primitiveJson.contains("material") && primitiveJson["material"].is_object()) {
				const auto& materialJson = primitiveJson["material"];
				if (materialJson.contains("color") && materialJson["color"].is_array() && materialJson["color"].size() == 4) {
					material.color = {materialJson["color"][0].get<float>(), materialJson["color"][1].get<float>(), materialJson["color"][2].get<float>(), materialJson["color"][3].get<float>()};
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
				if (materialJson.contains("distortionStrength") && materialJson["distortionStrength"].is_number()) {
					material.distortionStrength = materialJson["distortionStrength"].get<float>();
				}
				if (materialJson.contains("distortionFalloff") && materialJson["distortionFalloff"].is_number()) {
					material.distortionFalloff = materialJson["distortionFalloff"].get<float>();
				}
				if (materialJson.contains("uvScale") && materialJson["uvScale"].is_array() && materialJson["uvScale"].size() == 3) {
					material.uvScale = {materialJson["uvScale"][0].get<float>(), materialJson["uvScale"][1].get<float>(), materialJson["uvScale"][2].get<float>()};
				}
				if (materialJson.contains("uvRotate") && materialJson["uvRotate"].is_array() && materialJson["uvRotate"].size() == 3) {
					material.uvRotate = {materialJson["uvRotate"][0].get<float>(), materialJson["uvRotate"][1].get<float>(), materialJson["uvRotate"][2].get<float>()};
				}
				if (materialJson.contains("uvTranslate") && materialJson["uvTranslate"].is_array() && materialJson["uvTranslate"].size() == 3) {
					material.uvTranslate = {materialJson["uvTranslate"][0].get<float>(), materialJson["uvTranslate"][1].get<float>(), materialJson["uvTranslate"][2].get<float>()};
				}
				if (materialJson.contains("uvAnchor") && materialJson["uvAnchor"].is_array() && materialJson["uvAnchor"].size() == 2) {
					material.uvAnchor = {materialJson["uvAnchor"][0].get<float>(), materialJson["uvAnchor"][1].get<float>()};
				}
			}
			primitiveEditorMaterials_[index] = material;
			primitives_[index]->SetColor(material.color);
			primitives_[index]->SetEnableLighting(material.enableLighting);
			primitives_[index]->SetShininess(material.shininess);
			primitives_[index]->SetEnvironmentCoefficient(material.environmentCoefficient);
			primitives_[index]->SetGrayscaleEnabled(material.grayscaleEnabled);
			primitives_[index]->SetSepiaEnabled(material.sepiaEnabled);
			primitives_[index]->SetDistortionStrength(material.distortionStrength);
			primitives_[index]->SetDistortionFalloff(material.distortionFalloff);
			primitives_[index]->SetUvTransform(material.uvScale, material.uvRotate, material.uvTranslate, material.uvAnchor);
		}
	}

	return true;
}
void Hinstance::DrawSceneSelector() {
	SceneManager* sceneManager = SceneManager::GetInstance();
	if (!sceneManager) {
		return;
	}
	const std::vector<std::string> sceneNames = sceneManager->GetSceneNames();
	if (sceneNames.empty()) {
		ImGui::TextUnformatted("No scene list available");
		return;
	}

	const std::string& currentScene = sceneManager->GetCurrentSceneName();
	if (ImGui::BeginCombo("Scene", currentScene.empty() ? "(none)" : currentScene.c_str())) {
		for (const std::string& sceneName : sceneNames) {
			const bool isSelected = (sceneName == currentScene);
			if (ImGui::Selectable(sceneName.c_str(), isSelected) && !isSelected) {
				sceneManager->ChangeScene(sceneName);
				hasUnsavedChanges_ = false;
				saveStatusMessage_ = "Scene changed: " + sceneName;
			}
			if (isSelected) {
				ImGui::SetItemDefaultFocus();
			}
		}
		ImGui::EndCombo();
	}
}

void Hinstance::DrawGridEditor() {
	ImGui::Checkbox("Enable Grid Snap", &enableGridSnap_);
	if (enableGridSnap_) {
		ImGui::DragFloat("Grid Snap Spacing", &gridSnapSpacing_, 0.05f, 0.1f, 100.0f, "%.2f");
	}
}
void Hinstance::SetPlayMode(bool isPlaying) { isPlaying_ = isPlaying; }

void Hinstance::DrawObjectEditors() {
#ifdef USE_IMGUI

	if (!isPlaying_) {
		for (size_t i = 0; i < objects_.size(); ++i) {
			Object3d* object = objects_[i];
			if (!object) {
				continue;
			}
			const Transform& transform = editorTransforms_[i];
			const EditorMaterial& material = editorMaterials_[i];
			object->SetTransform(transform);
			object->SetColor(material.color);
			object->SetEnableLighting(material.enableLighting);
			object->SetShininess(material.shininess);
			object->SetEnvironmentCoefficient(material.environmentCoefficient);
			object->SetGrayscaleEnabled(material.grayscaleEnabled);
			object->SetSepiaEnabled(material.sepiaEnabled);
			object->SetDistortionStrength(material.distortionStrength);
			object->SetDistortionFalloff(material.distortionFalloff);
			object->SetUvTransform(material.uvScale, material.uvRotate, material.uvTranslate, material.uvAnchor);
		}

		for (size_t i = 0; i < primitives_.size(); ++i) {
			Primitive* primitive = primitives_[i];
			if (!primitive) {
				continue;
			}
			const Transform& transform = primitiveEditorTransforms_[i];
			const EditorMaterial& material = primitiveEditorMaterials_[i];
			primitive->SetTransform(transform);
			primitive->SetColor(material.color);
			primitive->SetEnableLighting(material.enableLighting);
			primitive->SetShininess(material.shininess);
			primitive->SetEnvironmentCoefficient(material.environmentCoefficient);
			primitive->SetGrayscaleEnabled(material.grayscaleEnabled);
			primitive->SetSepiaEnabled(material.sepiaEnabled);
			primitive->SetDistortionStrength(material.distortionStrength);
			primitive->SetDistortionFalloff(material.distortionFalloff);
			primitive->SetUvTransform(Function::MakeAffineMatrix(material.uvScale, material.uvRotate, material.uvTranslate,material.uvAnchor));
		}
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
	ImGui::SeparatorText("Scene Switch");
	DrawSceneSelector();
	ImGui::SeparatorText("Grid");
	DrawGridEditor();
	ImGui::Separator();

	if (!isPlaying_ && ImGui::Button("Save To JSON")) {
		const std::string saveFilePath = GetSceneScopedEditorFilePath("objectEditors.json");
		const bool saved = SaveObjectEditorsToJson(saveFilePath);
		if (saved) {
			hasUnsavedChanges_ = false;
		}
		saveStatusMessage_ = saved ? ("Saved: " + saveFilePath) : ("Save failed: " + saveFilePath);
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
			SetPlayMode(false);
			saveStatusMessage_ = "Stopped: applied editor values";
		}
	}

	ImGui::SeparatorText("Object3d");
	for (size_t i = 0; i < objects_.size(); ++i) {
		Object3d* object = objects_[i];
		if (!object) {
			continue;
		}
		std::string& objectName = objectNames_[i];
		std::string nodeLabel = objectName.empty() ? ("Object " + std::to_string(i)) : objectName;
		const std::string treeNodeLabel = nodeLabel + "###object_node_" + std::to_string(i);
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
				if (ImGui::InputText(("Name##object_" + std::to_string(i)).c_str(), nameBuffer.data(), nameBuffer.size())) {
					objectName = nameBuffer.data();
					nameChanged = true;
				}
				transformChanged |= ImGui::DragFloat3(("Scale##object_" + std::to_string(i)).c_str(), &transform.scale.x, 0.01f);
				transformChanged |= ImGui::DragFloat3(("Rotate##object_" + std::to_string(i)).c_str(), &transform.rotate.x, 0.01f);
				transformChanged |= ImGui::DragFloat3(("Translate##object_" + std::to_string(i)).c_str(), &transform.translate.x, 0.01f);
				ImGui::SeparatorText("Material");
				materialChanged |= ImGui::ColorEdit4(("Color##object_" + std::to_string(i)).c_str(), &material.color.x);
				materialChanged |= ImGui::Checkbox(("Enable Lighting##object_" + std::to_string(i)).c_str(), &material.enableLighting);
				materialChanged |= ImGui::DragFloat(("Shininess##object_" + std::to_string(i)).c_str(), &material.shininess, 0.1f, 0.0f, 256.0f);
				materialChanged |= ImGui::DragFloat(("Environment##object_" + std::to_string(i)).c_str(), &material.environmentCoefficient, 0.01f, 0.0f, 1.0f);
				materialChanged |= ImGui::Checkbox(("Grayscale##object_" + std::to_string(i)).c_str(), &material.grayscaleEnabled);
				materialChanged |= ImGui::Checkbox(("Sepia##object_" + std::to_string(i)).c_str(), &material.sepiaEnabled);
				materialChanged |= ImGui::DragFloat(("Distortion Strength##object_" + std::to_string(i)).c_str(), &material.distortionStrength, 0.01f, -10.0f, 10.0f);
				materialChanged |= ImGui::DragFloat(("Distortion Falloff##object_" + std::to_string(i)).c_str(), &material.distortionFalloff, 0.01f, 0.0f, 10.0f);
				ImGui::SeparatorText("UV Distortion");
				materialChanged |= ImGui::DragFloat3(("UV Scale##object_" + std::to_string(i)).c_str(), &material.uvScale.x, 0.01f);
				materialChanged |= ImGui::DragFloat3(("UV Rotate##object_" + std::to_string(i)).c_str(), &material.uvRotate.x, 0.01f);
				materialChanged |= ImGui::DragFloat3(("UV Translate##object_" + std::to_string(i)).c_str(), &material.uvTranslate.x, 0.01f);
				materialChanged |= ImGui::DragFloat2(("UV Anchor##object_" + std::to_string(i)).c_str(), &material.uvAnchor.x, 0.01f);
			}
			if (transformChanged || materialChanged || nameChanged) {
				hasUnsavedChanges_ = true;
			}
			if (transformChanged) {
				if (enableGridSnap_ && gridSnapSpacing_ > 0.0f) {
					transform.translate.x = std::round(transform.translate.x / gridSnapSpacing_) * gridSnapSpacing_;
					transform.translate.y = std::round(transform.translate.y / gridSnapSpacing_) * gridSnapSpacing_;
					transform.translate.z = std::round(transform.translate.z / gridSnapSpacing_) * gridSnapSpacing_;
				}
				object->SetTransform(transform);
			}
			if (materialChanged) {
				object->SetColor(material.color);
				object->SetEnableLighting(material.enableLighting);
				object->SetShininess(material.shininess);
				object->SetEnvironmentCoefficient(material.environmentCoefficient);
				object->SetGrayscaleEnabled(material.grayscaleEnabled);
				object->SetSepiaEnabled(material.sepiaEnabled);
				object->SetDistortionStrength(material.distortionStrength);
				object->SetDistortionFalloff(material.distortionFalloff);
				object->SetUvTransform(material.uvScale, material.uvRotate, material.uvTranslate, material.uvAnchor);
			}
			ImGui::TreePop();
		}
	}

	ImGui::SeparatorText("Primitive");
	for (size_t i = 0; i < primitives_.size(); ++i) {
		Primitive* primitive = primitives_[i];
		if (!primitive) {
			continue;
		}
		std::string& primitiveName = primitiveNames_[i];
		std::string nodeLabel = primitiveName.empty() ? ("Primitive " + std::to_string(i)) : primitiveName;
		const std::string treeNodeLabel = nodeLabel + "###primitive_node_" + std::to_string(i);
		if (ImGui::TreeNode(treeNodeLabel.c_str())) {
			Transform& transform = primitiveEditorTransforms_[i];
			EditorMaterial& material = primitiveEditorMaterials_[i];
			bool transformChanged = false;
			bool materialChanged = false;
			bool nameChanged = false;
			if (isPlaying_) {
				ImGui::TextUnformatted("Playing... editor values are locked");
			} else {
				std::array<char, 128> nameBuffer{};
				const size_t copyLength = std::min(nameBuffer.size() - 1, primitiveName.size());
				std::copy_n(primitiveName.begin(), copyLength, nameBuffer.begin());
				if (ImGui::InputText(("Name##primitive_" + std::to_string(i)).c_str(), nameBuffer.data(), nameBuffer.size())) {
					primitiveName = nameBuffer.data();
					nameChanged = true;
				}
				transformChanged |= ImGui::DragFloat3(("Scale##primitive_" + std::to_string(i)).c_str(), &transform.scale.x, 0.01f);
				transformChanged |= ImGui::DragFloat3(("Rotate##primitive_" + std::to_string(i)).c_str(), &transform.rotate.x, 0.01f);
				transformChanged |= ImGui::DragFloat3(("Translate##primitive_" + std::to_string(i)).c_str(), &transform.translate.x, 0.01f);
				ImGui::SeparatorText("Material");
				materialChanged |= ImGui::ColorEdit4(("Color##primitive_" + std::to_string(i)).c_str(), &material.color.x);
				materialChanged |= ImGui::Checkbox(("Enable Lighting##primitive_" + std::to_string(i)).c_str(), &material.enableLighting);
				materialChanged |= ImGui::DragFloat(("Shininess##primitive_" + std::to_string(i)).c_str(), &material.shininess, 0.1f, 0.0f, 256.0f);
				materialChanged |= ImGui::DragFloat(("Environment##primitive_" + std::to_string(i)).c_str(), &material.environmentCoefficient, 0.01f, 0.0f, 1.0f);
				materialChanged |= ImGui::Checkbox(("Grayscale##primitive_" + std::to_string(i)).c_str(), &material.grayscaleEnabled);
				materialChanged |= ImGui::Checkbox(("Sepia##primitive_" + std::to_string(i)).c_str(), &material.sepiaEnabled);
				materialChanged |= ImGui::DragFloat(("Distortion Strength##primitive_" + std::to_string(i)).c_str(), &material.distortionStrength, 0.01f, -10.0f, 10.0f);
				materialChanged |= ImGui::DragFloat(("Distortion Falloff##primitive_" + std::to_string(i)).c_str(), &material.distortionFalloff, 0.01f, 0.0f, 10.0f);
				ImGui::SeparatorText("UV Distortion");
				materialChanged |= ImGui::DragFloat3(("UV Scale##primitive_" + std::to_string(i)).c_str(), &material.uvScale.x, 0.01f);
				materialChanged |= ImGui::DragFloat3(("UV Rotate##primitive_" + std::to_string(i)).c_str(), &material.uvRotate.x, 0.01f);
				materialChanged |= ImGui::DragFloat3(("UV Translate##primitive_" + std::to_string(i)).c_str(), &material.uvTranslate.x, 0.01f);
				materialChanged |= ImGui::DragFloat2(("UV Anchor##primitive_" + std::to_string(i)).c_str(), &material.uvAnchor.x, 0.01f);
			}
			if (transformChanged || materialChanged || nameChanged) {
				hasUnsavedChanges_ = true;
			}
			if (transformChanged) {
				if (enableGridSnap_ && gridSnapSpacing_ > 0.0f) {
					transform.translate.x = std::round(transform.translate.x / gridSnapSpacing_) * gridSnapSpacing_;
					transform.translate.y = std::round(transform.translate.y / gridSnapSpacing_) * gridSnapSpacing_;
					transform.translate.z = std::round(transform.translate.z / gridSnapSpacing_) * gridSnapSpacing_;
				}
				primitive->SetTransform(transform);
			}
			if (materialChanged) {
				primitive->SetColor(material.color);
				primitive->SetEnableLighting(material.enableLighting);
				primitive->SetShininess(material.shininess);
				primitive->SetEnvironmentCoefficient(material.environmentCoefficient);
				primitive->SetGrayscaleEnabled(material.grayscaleEnabled);
				primitive->SetSepiaEnabled(material.sepiaEnabled);
				primitive->SetDistortionStrength(material.distortionStrength);
				primitive->SetDistortionFalloff(material.distortionFalloff);
				primitive->SetUvTransform(material.uvScale, material.uvRotate, material.uvTranslate, material.uvAnchor);
			}
			ImGui::TreePop();
		}
	}

	ImGui::End();
#endif
}