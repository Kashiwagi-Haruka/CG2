#define NOMINMAX
#include "Hierarchy.h"
#include "EditorGrid.h"
#include "ToolBar.h"
#include "Camera.h"
#include "Engine/BaseScene/SceneManager.h"
#include "Engine/Audio/Audio.h"
#include "Engine/Loadfile/JSON/JsonManager.h"
#include "Function.h"
#include "Object3d/Object3d.h"
#include "Object3d/Object3dCommon.h"
#include "Primitive/Primitive.h"
#ifdef USE_IMGUI
#include "externals/imgui/imgui.h"
#endif

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <filesystem>
#include <string>

namespace {
std::filesystem::path ResolveObjectEditorJsonPath(const std::string& filePath) { return std::filesystem::path("Resources") / "JSON" / std::filesystem::path(filePath).filename(); }

bool HasObjectEditorJsonFile(const std::string& filePath) { return std::filesystem::exists(ResolveObjectEditorJsonPath(filePath)); }
constexpr uint32_t kMaxPointLights = 20;
constexpr uint32_t kMaxSpotLights = 20;
constexpr uint32_t kMaxAreaLights = 20;
} // namespace

Hierarchy* Hierarchy::GetInstance() {
	static Hierarchy instance;
	return &instance;
}
void Hierarchy::Finalize() {
	objects_.clear();
	objectNames_.clear();
	editorTransforms_.clear();
	editorMaterials_.clear();

	primitives_.clear();
	primitiveNames_.clear();
	primitiveEditorTransforms_.clear();
	primitiveEditorMaterials_.clear();

	selectionBoxPrimitive_.reset();
	editorGridPlane_.reset();

	selectedObjectIndex_ = 0;
	selectedIsPrimitive_ = false;
	selectionBoxDirty_ = true;
	editorGridDirty_ = true;
	loadedSceneName_.clear();
	ResetForSceneChange();
}
std::string Hierarchy::GetSceneScopedEditorFilePath(const std::string& defaultFilePath) const {
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

void Hierarchy::ResetForSceneChange() {
	hasUnsavedChanges_ = false;
	saveStatusMessage_.clear();
	hasLoadedForCurrentScene_ = false;
	undoStack_.clear();
	redoStack_.clear();
	savedAudioVolumes_.clear();
	editorLightState_ = {};
}
void Hierarchy::ApplyEditorSnapshot(const EditorSnapshot& snapshot) {
	editorTransforms_ = snapshot.objectTransforms;
	editorMaterials_ = snapshot.objectMaterials;
	objectNames_ = snapshot.objectNames;
	primitiveEditorTransforms_ = snapshot.primitiveTransforms;
	primitiveEditorMaterials_ = snapshot.primitiveMaterials;
	primitiveNames_ = snapshot.primitiveNames;
	selectionBoxDirty_ = true;
}

void Hierarchy::UndoEditorChange() {
	if (undoStack_.empty()) {
		return;
	}
	EditorSnapshot current{};
	current.objectTransforms = editorTransforms_;
	current.objectMaterials = editorMaterials_;
	current.objectNames = objectNames_;
	current.primitiveTransforms = primitiveEditorTransforms_;
	current.primitiveMaterials = primitiveEditorMaterials_;
	current.primitiveNames = primitiveNames_;
	redoStack_.push_back(std::move(current));
	ApplyEditorSnapshot(undoStack_.back());
	undoStack_.pop_back();
	hasUnsavedChanges_ = true;
}

void Hierarchy::RedoEditorChange() {
	if (redoStack_.empty()) {
		return;
	}
	EditorSnapshot current{};
	current.objectTransforms = editorTransforms_;
	current.objectMaterials = editorMaterials_;
	current.objectNames = objectNames_;
	current.primitiveTransforms = primitiveEditorTransforms_;
	current.primitiveMaterials = primitiveEditorMaterials_;
	current.primitiveNames = primitiveNames_;
	undoStack_.push_back(std::move(current));
	ApplyEditorSnapshot(redoStack_.back());
	redoStack_.pop_back();
	hasUnsavedChanges_ = true;
}
void Hierarchy::RegisterObject3d(Object3d* object) {
	if (!object) {
		return;
	}
	if (std::find(objects_.begin(), objects_.end(), object) != objects_.end()) {
		return;
	}
	auto emptyIt = std::find(objects_.begin(), objects_.end(), nullptr);
	if (emptyIt != objects_.end()) {
		const size_t index = static_cast<size_t>(std::distance(objects_.begin(), emptyIt));
		objects_[index] = object;
		object->SetTransform(editorTransforms_[index]);
		const InspectorMaterial& material = editorMaterials_[index];
		object->SetColor(material.color);
		object->SetEnableLighting(material.enableLighting);
		object->SetShininess(material.shininess);
		object->SetEnvironmentCoefficient(material.environmentCoefficient);
		object->SetGrayscaleEnabled(material.grayscaleEnabled);
		object->SetSepiaEnabled(material.sepiaEnabled);
		object->SetDistortionStrength(material.distortionStrength);
		object->SetDistortionFalloff(material.distortionFalloff);
		object->SetUvTransform(material.uvScale, material.uvRotate, material.uvTranslate, material.uvAnchor);
		return;
	}
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

void Hierarchy::UnregisterObject3d(Object3d* object) {
	if (!object) {
		return;
	}
	for (size_t i = 0; i < objects_.size(); ++i) {
		if (objects_[i] == object) {
			objects_[i] = nullptr;
			if (!selectedIsPrimitive_ && selectedObjectIndex_ == i) {
				selectedObjectIndex_ = 0;
			}
			break;
		}
	}
}

void Hierarchy::RegisterPrimitive(Primitive* primitive) {
	if (!primitive) {
		return;
	}
	if (std::find(primitives_.begin(), primitives_.end(), primitive) != primitives_.end()) {
		return;
	}
	auto emptyIt = std::find(primitives_.begin(), primitives_.end(), nullptr);
	if (emptyIt != primitives_.end()) {
		const size_t index = static_cast<size_t>(std::distance(primitives_.begin(), emptyIt));
		primitives_[index] = primitive;
		primitive->SetTransform(primitiveEditorTransforms_[index]);
		const InspectorMaterial& material = primitiveEditorMaterials_[index];
		primitive->SetColor(material.color);
		primitive->SetEnableLighting(material.enableLighting);
		primitive->SetShininess(material.shininess);
		primitive->SetEnvironmentCoefficient(material.environmentCoefficient);
		primitive->SetGrayscaleEnabled(material.grayscaleEnabled);
		primitive->SetSepiaEnabled(material.sepiaEnabled);
		primitive->SetDistortionStrength(material.distortionStrength);
		primitive->SetDistortionFalloff(material.distortionFalloff);
		primitive->SetUvTransform(material.uvScale, material.uvRotate, material.uvTranslate, material.uvAnchor);
		return;
	}
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

void Hierarchy::UnregisterPrimitive(Primitive* primitive) {
	if (!primitive) {
		return;
	}
	for (size_t i = 0; i < primitives_.size(); ++i) {
		if (primitives_[i] == primitive) {
			primitives_[i] = nullptr;
			if (selectedIsPrimitive_ && selectedObjectIndex_ == i) {
				selectedObjectIndex_ = 0;
			}
			break;
		}
	}
}

bool Hierarchy::HasRegisteredObjects() const { return !objects_.empty() || !primitives_.empty(); }

bool Hierarchy::LoadObjectEditorsFromJsonIfExists(const std::string& filePath) {
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
	if (hasLoadedForCurrentScene_) {
		return true;
	}
	hasLoadedForCurrentScene_ = LoadObjectEditorsFromJson(scopedFilePath);
	return hasLoadedForCurrentScene_;
}

bool Hierarchy::SaveObjectEditorsToJson(const std::string& filePath) const {
	nlohmann::json root;
	root["objects"] = nlohmann::json::array();
	root["primitives"] = nlohmann::json::array();
	root["lights"] = nlohmann::json::object();
	root["audio"] = {
	    {"sounds", nlohmann::json::array()}
    };
	for (size_t i = 0; i < objects_.size(); ++i) {
		const Object3d* object = objects_[i];
		if (!object) {
			continue;
		}
		const Transform& transform = editorTransforms_[i];
		const InspectorMaterial& material = editorMaterials_[i];
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
		if (!primitive || primitive == selectionBoxPrimitive_.get()) {
			continue;
		}
		const Transform& transform = primitiveEditorTransforms_[i];
		const InspectorMaterial& material = editorMaterials_[i];
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
	Audio* audio = Audio::GetInstance();
	if (audio) {
		for (const auto& entry : audio->GetEditorSoundEntries()) {
			if (!entry.soundData || entry.name.empty()) {
				continue;
			}
			root["audio"]["sounds"].push_back({
			    {"name",   entry.name             },
			    {"volume", entry.soundData->volume},
			});
		}
	}
	JsonManager* jsonManager = JsonManager::GetInstance();
	jsonManager->SetData(root);
	return jsonManager->SaveJson(filePath);
}

bool Hierarchy::LoadObjectEditorsFromJson(const std::string& filePath) {
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
			InspectorMaterial material = editorMaterials_[index];
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
			if (index >= primitives_.size() || !primitives_[index] || primitives_[index] == selectionBoxPrimitive_.get()) {
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
			InspectorMaterial material = primitiveEditorMaterials_[index];
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

	Object3dCommon* object3dCommon = Object3dCommon::GetInstance();
	if (object3dCommon) {
		editorLightState_.directionalLight = object3dCommon->GetDirectionalLightSource();
		editorLightState_.pointLights = object3dCommon->GetPointLightSource();
		editorLightState_.spotLights = object3dCommon->GetSpotLightSource();
		editorLightState_.areaLights = object3dCommon->GetAreaLightSource();
	}
	if (root.contains("audio") && root["audio"].is_object()) {
		const auto& audioJson = root["audio"];
		if (audioJson.contains("sounds") && audioJson["sounds"].is_array()) {
			for (const auto& soundJson : audioJson["sounds"]) {
				if (!soundJson.is_object()) {
					continue;
				}
				if (!soundJson.contains("name") || !soundJson["name"].is_string()) {
					continue;
				}
				if (!soundJson.contains("volume") || !soundJson["volume"].is_number()) {
					continue;
				}
				const std::string name = soundJson["name"].get<std::string>();
				const float volume = std::clamp(soundJson["volume"].get<float>(), 0.0f, 1.0f);
				savedAudioVolumes_[name] = volume;
			}
		}
	}
	return true;
}
void Hierarchy::DrawSceneSelector() {
#ifdef USE_IMGUI
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
#else
	SceneManager* sceneManager = SceneManager::GetInstance();
	if (!sceneManager) {
		return;
	}
#endif
}

void Hierarchy::DrawGridEditor() {
#ifdef USE_IMGUI
	ImGui::Checkbox("Enable Grid Snap", &enableGridSnap_);
	if (ImGui::DragFloat("Grid Snap Spacing", &gridSnapSpacing_, 0.05f, 0.1f, 100.0f, "%.2f")) {
		editorGridDirty_ = true;
	}
	if (ImGui::Checkbox("Draw Editor Grid Lines", &showEditorGridLines_)) {
		editorGridDirty_ = true;
	}
	if (ImGui::DragInt("Grid Half Line Count", &gridHalfLineCount_, 1.0f, 1, 200)) {
		editorGridDirty_ = true;
	}
	if (ImGui::DragFloat("Grid Y", &editorGridY_, 0.01f, -100.0f, 100.0f, "%.2f")) {
		editorGridDirty_ = true;
	}
#endif

	gridSnapSpacing_ = std::max(gridSnapSpacing_, 0.1f);
	gridHalfLineCount_ = std::max(gridHalfLineCount_, 1);
}
void Hierarchy::SetPlayMode(bool isPlaying) { isPlaying_ = isPlaying; }

void Hierarchy::DrawEditorGridLines() {
#ifdef USE_IMGUI
	if (!showEditorGridLines_) {
		return;
	}

	if (editorGridDirty_ || !editorGridPlane_) {
		if (!editorGridPlane_) {
			editorGridPlane_ = std::make_unique<Primitive>();
			editorGridPlane_->SetEditorRegistrationEnabled(false);
			editorGridPlane_->Initialize(Primitive::Plane);
			editorGridPlane_->SetCamera(Object3dCommon::GetInstance()->GetDefaultCamera());
			editorGridPlane_->SetEnableLighting(false);
		}

		const float extent = static_cast<float>(gridHalfLineCount_) * gridSnapSpacing_;
		Transform gridTransform{};
		gridTransform.scale = {extent * 2.0f, extent * 2.0f, 1.0f};
		gridTransform.rotate = {Function::kPi * 0.5f, 0.0f, 0.0f};
		gridTransform.translate = {0.0f, editorGridY_, 0.0f};
		editorGridPlane_->SetTransform(gridTransform);
		editorGridPlane_->SetColor({1.0f, 1.0f, 1.0f, 1.0f});
		editorGridPlane_->SetDistortionFalloff(gridSnapSpacing_);                        // spacing
		editorGridPlane_->SetDistortionStrength(static_cast<float>(gridHalfLineCount_)); // half line count
		editorGridPlane_->SetEnvironmentCoefficient(gridSnapSpacing_ * 0.025f);          // line width in world unit
		editorGridDirty_ = false;
	}

	if (!editorGridPlane_) {
		return;
	}

	// シーン切り替え後にカメラが再生成されるため、毎フレーム最新のカメラを参照する
	editorGridPlane_->SetCamera(Object3dCommon::GetInstance()->GetDefaultCamera());

	Object3dCommon::GetInstance()->DrawCommonEditorGrid();
	editorGridPlane_->Update();
	editorGridPlane_->Draw();
	if (!showSelectionBox_ || !IsObjectSelected()) {
		return;
	}
	if (selectionBoxDirty_ || !selectionBoxPrimitive_) {
		SyncSelectionBoxToTarget();
		selectionBoxDirty_ = false;
	}
	if (!selectionBoxPrimitive_) {
		return;
	}
	selectionBoxPrimitive_->SetCamera(Object3dCommon::GetInstance()->GetDefaultCamera());
	Object3dCommon::GetInstance()->DrawCommonWireframeNoDepth();
	selectionBoxPrimitive_->Update();
	selectionBoxPrimitive_->Draw();
#endif
}
void Hierarchy::DrawCameraEditor() {
#ifdef USE_IMGUI
	Object3dCommon* object3dCommon = Object3dCommon::GetInstance();
	if (!object3dCommon) {
		ImGui::TextUnformatted("Object3dCommon unavailable");
		return;
	}
	Camera* camera = object3dCommon->GetDefaultCamera();
	if (!camera) {
		ImGui::TextUnformatted("No default camera");
		return;
	}
	camera->DrawEditorInHierarchy();
#endif
}
void Hierarchy::DrawLightEditor() {
#ifdef USE_IMGUI
	Object3dCommon* object3dCommon = Object3dCommon::GetInstance();
	if (!object3dCommon) {
		ImGui::TextUnformatted("Object3dCommon unavailable");
		return;
	}
	editorLightState_.directionalLight = object3dCommon->GetDirectionalLightSource();
	editorLightState_.pointLights = object3dCommon->GetPointLightSource();
	editorLightState_.spotLights = object3dCommon->GetSpotLightSource();
	editorLightState_.areaLights = object3dCommon->GetAreaLightSource();

	bool lightChanged = false;
	if (ImGui::TreeNode("Directional Light")) {
		if (!editorLightState_.directionalLight) {
			ImGui::TextUnformatted("No directional light source");
		} else if (!isPlaying_) {
			Vector4 color = editorLightState_.directionalLight->GetColor();
			if (ImGui::ColorEdit4("Dir Color", &color.x)) {
				editorLightState_.directionalLight->SetColor(color);
				lightChanged = true;
			}
			Vector3 direction = editorLightState_.directionalLight->GetDirection();
			if (ImGui::DragFloat3("Dir Direction", &direction.x, 0.01f, -1.0f, 1.0f)) {
				editorLightState_.directionalLight->SetDirection(direction);
				lightChanged = true;
			}
			float intensity = editorLightState_.directionalLight->GetIntensity();
			if (ImGui::DragFloat("Dir Intensity", &intensity, 0.01f, 0.0f, 10.0f)) {
				editorLightState_.directionalLight->SetIntensity(intensity);
				lightChanged = true;
			}
		}
		ImGui::TreePop();
	}

	if (ImGui::TreeNode("Point Lights")) {
		if (!editorLightState_.pointLights) {
			ImGui::TextUnformatted("No point light source");
		} else {
			int pointCount = editorLightState_.pointLights->GetLightCount();
			if (!isPlaying_ && ImGui::SliderInt("Point Count", &pointCount, 0, static_cast<int>(kMaxPointLights))) {
				int current = editorLightState_.pointLights->GetLightCount();
				while (current < pointCount) {
					editorLightState_.pointLights->AddPointLight("Point" + std::to_string(current++));
				}
				while (current > pointCount) {
					editorLightState_.pointLights->RemovePointLight("Point" + std::to_string(--current));
				}
				lightChanged = true;
			}
			for (int i = 0; i < editorLightState_.pointLights->GetLightCount(); ++i) {
				std::string n = "Point" + std::to_string(i);
				if (ImGui::TreeNode((n + "##point").c_str())) {
					if (!isPlaying_) {
						Vector4 c = editorLightState_.pointLights->GetLightColor(n);
						Vector3 p = editorLightState_.pointLights->GetLightPosition(n);
						float in = editorLightState_.pointLights->GetLightIntensity(n);
						float r = editorLightState_.pointLights->GetLightRadius(n);
						float d = editorLightState_.pointLights->GetLightDecay(n);
						lightChanged |= ImGui::ColorEdit4(("Color##" + n).c_str(), &c.x);
						lightChanged |= ImGui::DragFloat3(("Position##" + n).c_str(), &p.x, 0.05f);
						lightChanged |= ImGui::DragFloat(("Intensity##" + n).c_str(), &in, 0.01f, 0.0f, 10.0f);
						lightChanged |= ImGui::DragFloat(("Radius##" + n).c_str(), &r, 0.05f, 0.0f, 500.0f);
						lightChanged |= ImGui::DragFloat(("Decay##" + n).c_str(), &d, 0.01f, 0.0f, 10.0f);
						editorLightState_.pointLights->SetLightProperties(n, c, p, in, r, d);
					}
					ImGui::TreePop();
				}
			}
		}
		ImGui::TreePop();
	}

	if (ImGui::TreeNode("Spot Lights")) {
		if (!editorLightState_.spotLights) {
			ImGui::TextUnformatted("No spot light source");
		} else {
			int spotCount = editorLightState_.spotLights->GetSpotLightCount();
			if (!isPlaying_ && ImGui::SliderInt("Spot Count", &spotCount, 0, static_cast<int>(kMaxSpotLights))) {
				int current = editorLightState_.spotLights->GetSpotLightCount();
				while (current < spotCount) {
					editorLightState_.spotLights->AddSpotLight("Spot" + std::to_string(current++));
				}
				while (current > spotCount) {
					editorLightState_.spotLights->RemoveSpotLight("Spot" + std::to_string(--current));
				}
				lightChanged = true;
			}
			for (int i = 0; i < editorLightState_.spotLights->GetSpotLightCount(); ++i) {
				std::string n = "Spot" + std::to_string(i);
				if (ImGui::TreeNode((n + "##spot").c_str())) {
					if (!isPlaying_) {
						Vector4 c = editorLightState_.spotLights->GetSpotLightColor(n);
						Vector3 p = editorLightState_.spotLights->GetSpotLightPosition(n);
						Vector3 dir = editorLightState_.spotLights->GetSpotLightDirection(n);
						float in = editorLightState_.spotLights->GetSpotLightIntensity(n);
						float dist = editorLightState_.spotLights->GetSpotLightDistance(n);
						float dec = editorLightState_.spotLights->GetSpotLightDecay(n);
						float ang = editorLightState_.spotLights->GetSpotLightAngle(n);
						float fall = editorLightState_.spotLights->GetSpotLightFalloffStartAngle(n);
						lightChanged |= ImGui::ColorEdit4(("Color##" + n).c_str(), &c.x);
						lightChanged |= ImGui::DragFloat3(("Position##" + n).c_str(), &p.x, 0.05f);
						lightChanged |= ImGui::DragFloat3(("Direction##" + n).c_str(), &dir.x, 0.01f, -1.0f, 1.0f);
						lightChanged |= ImGui::DragFloat(("Intensity##" + n).c_str(), &in, 0.01f, 0.0f, 10.0f);
						lightChanged |= ImGui::DragFloat(("Distance##" + n).c_str(), &dist, 0.05f, 0.0f, 500.0f);
						lightChanged |= ImGui::DragFloat(("Decay##" + n).c_str(), &dec, 0.01f, 0.0f, 10.0f);
						lightChanged |= ImGui::DragFloat(("Angle##" + n).c_str(), &ang, 0.001f, 0.0f, 3.14159f);
						lightChanged |= ImGui::DragFloat(("Falloff##" + n).c_str(), &fall, 0.001f, 0.0f, 3.14159f);
						editorLightState_.spotLights->SetSpotLightProperties(n, c, p, in, dir, dist, dec, ang, fall);
					}
					ImGui::TreePop();
				}
			}
		}
		ImGui::TreePop();
	}

	if (ImGui::TreeNode("Area Lights")) {
		if (!editorLightState_.areaLights) {
			ImGui::TextUnformatted("No area light source");
		} else {
			int areaCount = editorLightState_.areaLights->GetAreaLightCount();
			if (!isPlaying_ && ImGui::SliderInt("Area Count", &areaCount, 0, static_cast<int>(kMaxAreaLights))) {
				int current = editorLightState_.areaLights->GetAreaLightCount();
				while (current < areaCount) {
					editorLightState_.areaLights->AddAreaLight("Area" + std::to_string(current++));
				}
				while (current > areaCount) {
					editorLightState_.areaLights->RemoveAreaLight("Area" + std::to_string(--current));
				}
				lightChanged = true;
			}
			for (int i = 0; i < editorLightState_.areaLights->GetAreaLightCount(); ++i) {
				std::string n = "Area" + std::to_string(i);
				if (ImGui::TreeNode((n + "##area").c_str())) {
					if (!isPlaying_) {
						Vector4 c = editorLightState_.areaLights->GetAreaLightColor(n);
						Vector3 p = editorLightState_.areaLights->GetAreaLightPosition(n);
						Vector3 norm = editorLightState_.areaLights->GetAreaLightNormal(n);
						float in = editorLightState_.areaLights->GetAreaLightIntensity(n);
						Vector2 sz = editorLightState_.areaLights->GetAreaLightSize(n);
						float r = editorLightState_.areaLights->GetAreaLightDistance(n);
						float d = editorLightState_.areaLights->GetAreaLightDecay(n);
						lightChanged |= ImGui::ColorEdit4(("Color##" + n).c_str(), &c.x);
						lightChanged |= ImGui::DragFloat3(("Position##" + n).c_str(), &p.x, 0.05f);
						lightChanged |= ImGui::DragFloat3(("Normal##" + n).c_str(), &norm.x, 0.01f, -1.0f, 1.0f);
						lightChanged |= ImGui::DragFloat(("Intensity##" + n).c_str(), &in, 0.01f, 0.0f, 10.0f);
						lightChanged |= ImGui::DragFloat(("Width##" + n).c_str(), &sz.x, 0.05f, 0.0f, 500.0f);
						lightChanged |= ImGui::DragFloat(("Height##" + n).c_str(), &sz.y, 0.05f, 0.0f, 500.0f);
						lightChanged |= ImGui::DragFloat(("Radius##" + n).c_str(), &r, 0.05f, 0.0f, 500.0f);
						lightChanged |= ImGui::DragFloat(("Decay##" + n).c_str(), &d, 0.01f, 0.0f, 10.0f);
						editorLightState_.areaLights->SetAreaLightColor(n, c);
						editorLightState_.areaLights->SetAreaLightPosition(n, p);
						editorLightState_.areaLights->SetAreaLightNormal(n, norm);
						editorLightState_.areaLights->SetAreaLightIntensity(n, in);
						editorLightState_.areaLights->SetAreaLightSize(n, sz.x, sz.y);
						editorLightState_.areaLights->SetAreaLightDistance(n, r);
						editorLightState_.areaLights->SetAreaLightDecay(n, d);
					}
					ImGui::TreePop();
				}
			}
		}
		ImGui::TreePop();
	}

	if (lightChanged) {
		hasUnsavedChanges_ = true;
		if (editorLightState_.directionalLight) {
			object3dCommon->SetDirectionalLight(*editorLightState_.directionalLight);
		}
		if (editorLightState_.pointLights) {
			object3dCommon->SetPointLights(*editorLightState_.pointLights);
		}
		if (editorLightState_.spotLights) {
			object3dCommon->SetSpotLights(*editorLightState_.spotLights);
		}
		if (editorLightState_.areaLights) {
			object3dCommon->SetAreaLights(*editorLightState_.areaLights);
		}
	}
#endif
}
void Hierarchy::DrawAudioEditor() {
#ifdef USE_IMGUI
	Audio* audio = Audio::GetInstance();
	if (!audio) {
		ImGui::TextUnformatted("Audio system unavailable");
		return;
	}
	auto entries = audio->GetEditorSoundEntries();
	if (entries.empty()) {
		ImGui::TextUnformatted("No tracked sounds.");
		return;
	}
	for (size_t i = 0; i < entries.size(); ++i) {
		auto& entry = entries[i];
		if (!entry.soundData) {
			continue;
		}
		if (ImGui::TreeNode((entry.name + "##audio_" + std::to_string(i)).c_str())) {
			const auto savedIt = savedAudioVolumes_.find(entry.name);
			if (savedIt != savedAudioVolumes_.end()) {
				audio->SetSoundVolume(entry.soundData, savedIt->second);
			}
			float volume = entry.soundData->volume;
			if (ImGui::SliderFloat(("Volume##audio_volume_" + std::to_string(i)).c_str(), &volume, 0.0f, 1.0f)) {
				audio->SetSoundVolume(entry.soundData, volume);
				savedAudioVolumes_[entry.name] = volume;
				hasUnsavedChanges_ = true;
			}
			if (entry.isPlaying) {
				ImGui::Text("Playing (%s)", entry.isLoop ? "Loop" : "One-shot");
			} else {
				ImGui::TextUnformatted("Stopped");
			}
			if (ImGui::Button(("Play##audio_play_" + std::to_string(i)).c_str())) {
				audio->SoundPlayWave(*entry.soundData, false);
			}
			ImGui::SameLine();
			if (ImGui::Button(("Loop##audio_loop_" + std::to_string(i)).c_str())) {
				audio->SoundPlayWave(*entry.soundData, true);
			}
			ImGui::TreePop();
		}
	}
#endif
}

bool Hierarchy::IsObjectSelected() const {
	if (selectedIsPrimitive_) {
		return selectedObjectIndex_ < primitives_.size() && primitives_[selectedObjectIndex_] != nullptr;
	}
	return selectedObjectIndex_ < objects_.size() && objects_[selectedObjectIndex_] != nullptr;
}

Transform Hierarchy::GetSelectedTransform() const {
	if (selectedIsPrimitive_) {
		if (selectedObjectIndex_ < primitiveEditorTransforms_.size()) {
			return primitiveEditorTransforms_[selectedObjectIndex_];
		}
	} else {
		if (selectedObjectIndex_ < editorTransforms_.size()) {
			return editorTransforms_[selectedObjectIndex_];
		}
	}
	return Transform{};
}

void Hierarchy::SyncSelectionBoxToTarget() {
	if (!showSelectionBox_ || !IsObjectSelected()) {
		return;
	}
	if (!selectionBoxPrimitive_) {
		selectionBoxPrimitive_ = std::make_unique<Primitive>();
		selectionBoxPrimitive_->SetEditorRegistrationEnabled(false);
		selectionBoxPrimitive_->Initialize(Primitive::Box);
		selectionBoxPrimitive_->SetEnableLighting(false);
		selectionBoxPrimitive_->SetColor({1.0f, 0.9f, 0.1f, 1.0f});
	}
	selectionBoxPrimitive_->SetCamera(Object3dCommon::GetInstance()->GetDefaultCamera());
	const Transform selectedTransform = GetSelectedTransform();
	selectionBoxPrimitive_->SetTransform(selectedTransform);
}

void Hierarchy::DrawSelectionBoxEditor() {
#ifdef USE_IMGUI
	if (ImGui::Checkbox("Draw Selection Box", &showSelectionBox_)) {
		selectionBoxDirty_ = true;
	}
#endif
}
void Hierarchy::DrawObjectEditors() {
	LoadObjectEditorsFromJsonIfExists("objectEditors.json");
#ifdef USE_IMGUI

	if (!isPlaying_) {
		for (size_t i = 0; i < objects_.size(); ++i) {
			Object3d* object = objects_[i];
			if (!object) {
				continue;
			}
			const Transform& transform = editorTransforms_[i];
			const InspectorMaterial& material = editorMaterials_[i];
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
			const InspectorMaterial& material = primitiveEditorMaterials_[i];
			primitive->SetTransform(transform);
			primitive->SetColor(material.color);
			primitive->SetEnableLighting(material.enableLighting);
			primitive->SetShininess(material.shininess);
			primitive->SetEnvironmentCoefficient(material.environmentCoefficient);
			primitive->SetGrayscaleEnabled(material.grayscaleEnabled);
			primitive->SetSepiaEnabled(material.sepiaEnabled);
			primitive->SetDistortionStrength(material.distortionStrength);
			primitive->SetDistortionFalloff(material.distortionFalloff);
			primitive->SetUvTransform(Function::MakeAffineMatrix(material.uvScale, material.uvRotate, material.uvTranslate, material.uvAnchor));
		}
	}
	const ImGuiViewport* viewport = ImGui::GetMainViewport();
	const float kTopToolbarHeight = 44.0f;
	const float kLeftPanelRatio = 0.22f;
	const float kRightPanelRatio = 0.24f;
	const float kPanelMinWidth = 260.0f;
	const float availableHeight = std::max(1.0f, viewport->WorkSize.y - kTopToolbarHeight);
	const float leftPanelWidth = std::max(kPanelMinWidth, viewport->WorkSize.x * kLeftPanelRatio);
	const float rightPanelWidth = std::max(kPanelMinWidth, viewport->WorkSize.x * kRightPanelRatio);


	ImGui::SetNextWindowPos(ImVec2(viewport->WorkPos.x, viewport->WorkPos.y), ImGuiCond_Always);
	ImGui::SetNextWindowSize(ImVec2(viewport->WorkSize.x, kTopToolbarHeight), ImGuiCond_Always);
	if (ImGui::Begin("Toolbar", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse)) {
		ToolBar::Result toolbarResult = ToolBar::Draw(isPlaying_, hasUnsavedChanges_, !undoStack_.empty(), !redoStack_.empty());
		if (toolbarResult.undoRequested) {
			UndoEditorChange();
			saveStatusMessage_ = "Undo";
		}
		if (toolbarResult.redoRequested) {
			RedoEditorChange();
			saveStatusMessage_ = "Redo";
		}
		if (toolbarResult.playRequested) {
			if (hasUnsavedChanges_) {
				saveStatusMessage_ = "Warning: unsaved changes. Save To JSON before Play";
			} else {
				SceneManager::GetInstance()->RequestReinitializeCurrentScene();
				SetPlayMode(true);
				saveStatusMessage_ = "Playing";
			}
		}
		if (toolbarResult.stopRequested) {
			SetPlayMode(false);
			saveStatusMessage_ = "Stopped: applied editor values";
		}
	}
	ImGui::End();

	const float contentStartY = viewport->WorkPos.y + kTopToolbarHeight;
	ImGui::SetNextWindowPos(ImVec2(viewport->WorkPos.x, contentStartY), ImGuiCond_Always);
	ImGui::SetNextWindowSize(ImVec2(leftPanelWidth, availableHeight), ImGuiCond_Always);
	if (ImGui::Begin("Hierarchy", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize)) {
		ImGui::Text("Auto Object Editor");
		ImGui::Separator();
		ImGui::SeparatorText("Scene Switch");
		DrawSceneSelector();
		ImGui::SeparatorText("Grid");
		DrawGridEditor();
		ImGui::SeparatorText("Light");
		DrawLightEditor();
		ImGui::SeparatorText("Camera");
		DrawCameraEditor();
		ImGui::SeparatorText("Selection");
		DrawSelectionBoxEditor();
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

		ImGui::SeparatorText("Object3d");
		for (size_t i = 0; i < objects_.size(); ++i) {
			Object3d* object = objects_[i];
			if (!object) {
				continue;
			}
			std::string displayName = objectNames_[i].empty() ? ("Object " + std::to_string(i)) : objectNames_[i];
			const bool selected = (!selectedIsPrimitive_ && selectedObjectIndex_ == i);
			if (ImGui::Selectable((displayName + "##object_select_" + std::to_string(i)).c_str(), selected)) {
				selectedObjectIndex_ = i;
				selectedIsPrimitive_ = false;
				selectionBoxDirty_ = true;
			}
		}

		ImGui::SeparatorText("Primitive");
		for (size_t i = 0; i < primitives_.size(); ++i) {
			Primitive* primitive = primitives_[i];
			if (!primitive) {
				continue;
			}
			std::string displayName = primitiveNames_[i].empty() ? ("Primitive " + std::to_string(i)) : primitiveNames_[i];
			const bool selected = (selectedIsPrimitive_ && selectedObjectIndex_ == i);
			if (ImGui::Selectable((displayName + "##primitive_select_" + std::to_string(i)).c_str(), selected)) {
				selectedObjectIndex_ = i;
				selectedIsPrimitive_ = true;
				selectionBoxDirty_ = true;
			}
		}
	}
	ImGui::End();

	const float inspectorPosX = viewport->WorkPos.x + viewport->WorkSize.x - rightPanelWidth;
	ImGui::SetNextWindowPos(ImVec2(inspectorPosX, contentStartY), ImGuiCond_Always);
	ImGui::SetNextWindowSize(ImVec2(rightPanelWidth, availableHeight), ImGuiCond_Always);
	if (ImGui::Begin("Inspector", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize)) {
		if (!IsObjectSelected()) {
			ImGui::TextUnformatted("No object selected.");
		} else {
			EditorSnapshot beforeEdit{};
			beforeEdit.objectTransforms = editorTransforms_;
			beforeEdit.objectMaterials = editorMaterials_;
			beforeEdit.objectNames = objectNames_;
			beforeEdit.primitiveTransforms = primitiveEditorTransforms_;
			beforeEdit.primitiveMaterials = primitiveEditorMaterials_;
			beforeEdit.primitiveNames = primitiveNames_;
			bool transformChanged = false;
			bool materialChanged = false;
			bool nameChanged = false;
			if (selectedIsPrimitive_) {
				Primitive* primitive = primitives_[selectedObjectIndex_];
				if (primitive) {
					Transform& transform = primitiveEditorTransforms_[selectedObjectIndex_];
					InspectorMaterial& material = primitiveEditorMaterials_[selectedObjectIndex_];
					std::string& name = primitiveNames_[selectedObjectIndex_];
					Inspector::DrawPrimitiveInspector(selectedObjectIndex_, name, transform, material, isPlaying_, transformChanged, materialChanged, nameChanged);
					if (transformChanged) {
						selectionBoxDirty_ = true;
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
				}
			} else {
				Object3d* object = objects_[selectedObjectIndex_];
				if (object) {
					Transform& transform = editorTransforms_[selectedObjectIndex_];
					InspectorMaterial& material = editorMaterials_[selectedObjectIndex_];
					std::string& name = objectNames_[selectedObjectIndex_];
					Inspector::DrawObjectInspector(selectedObjectIndex_, name, transform, material, isPlaying_, transformChanged, materialChanged, nameChanged);
					if (transformChanged) {
						selectionBoxDirty_ = true;
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
				}
			}
			if (transformChanged || materialChanged || nameChanged) {
				undoStack_.push_back(std::move(beforeEdit));
				redoStack_.clear();
				hasUnsavedChanges_ = true;
			}
		}
	}
	ImGui::End();
#endif
}