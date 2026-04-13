#define NOMINMAX
#include "Hierarchy.h"
#include "Camera.h"
#include "Engine/BaseScene/SceneManager.h"
#include "Engine/Editor/EditorData/Object3d/EditorObject3d.h"
#include "Engine/Editor/EditorData/Primitive/EditorPrimitive.h"
#include "Engine/Editor/EditorTool/Grid/EditorGrid.h"
#include "Engine/Editor/EditorTool/ToolBar/ToolBar.h"
#include "Engine/Loadfile/JSON/JsonManager.h"
#include "Function.h"
#include "Input.h"
#include "Object3d/Object3d.h"
#include "Object3d/Object3dCommon.h"
#include "Primitive/Primitive.h"
#include "Sprite/SpriteCommon.h"
#ifdef USE_IMGUI
#include "externals/imgui/imgui.h"
#endif

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <filesystem>
#include <string>
#include <limits>
#include <string>
#include <unordered_map>

namespace {
std::filesystem::path ResolveObjectEditorJsonPath(const std::string& filePath) { return std::filesystem::path("Resources") / "JSON" / std::filesystem::path(filePath).filename(); }

bool HasObjectEditorJsonFile(const std::string& filePath) { return std::filesystem::exists(ResolveObjectEditorJsonPath(filePath)); }

} // namespace

Hierarchy* Hierarchy::GetInstance() {
	static Hierarchy instance;
	return &instance;
}
void Hierarchy::Finalize() {
	editorAudio_.Finalize();
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
	gridSettings_.dirty = true;
	loadedSceneName_.clear();
	editorCamera_.Reset();
	ResetForSceneChange();
}
bool Hierarchy::IsEditorPreviewActive() const {
#ifdef USE_IMGUI
	return HasRegisteredObjects() && !isPlaying_;
#else
	return false;
#endif
}

void Hierarchy::UpdateEditorPreview() {
#ifdef USE_IMGUI
	editorCamera_.UpdateEditorPreview(IsEditorPreviewActive(), objects_, primitives_);
#endif
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
	editorAudio_.ResetForSceneChange();
	hasUnsavedChanges_ = false;
	saveStatusMessage_.clear();
	hasLoadedForCurrentScene_ = false;
	hasLoadedSnapshot_ = false;
	loadedSnapshotFilePath_.clear();
	editorLight_.Reset();
	undoStack_.clear();
	redoStack_.clear();
	editorCamera_.DeactivatePreview();
}

Hierarchy::EditorSnapshot Hierarchy::CreateCurrentSnapshot() const {
	EditorSnapshot snapshot{};
	snapshot.objectTransforms = editorTransforms_;
	snapshot.objectMaterials = editorMaterials_;
	snapshot.objectNames = objectNames_;
	snapshot.primitiveTransforms = primitiveEditorTransforms_;
	snapshot.primitiveMaterials = primitiveEditorMaterials_;
	snapshot.primitiveNames = primitiveNames_;
	return snapshot;
}

bool Hierarchy::ResetToLoadedSnapshot() {
	if (!hasLoadedSnapshot_) {
		return false;
	}
	if (!loadedSnapshotFilePath_.empty() && HasObjectEditorJsonFile(loadedSnapshotFilePath_)) {
		if (!LoadObjectEditorsFromJson(loadedSnapshotFilePath_)) {
			return false;
		}
	} else {
		ApplyEditorSnapshot(loadedSnapshot_);
	}
	undoStack_.clear();
	redoStack_.clear();
	hasUnsavedChanges_ = false;
	return true;
}


void Hierarchy::UndoEditorChange() {
	if (undoStack_.empty()) {
		return;
	}
	EditorSnapshot current = CreateCurrentSnapshot();
	redoStack_.push_back(std::move(current));
	ApplyEditorSnapshot(undoStack_.back());
	undoStack_.pop_back();
	hasUnsavedChanges_ = true;
}

void Hierarchy::RedoEditorChange() {
	if (redoStack_.empty()) {
		return;
	}
	EditorSnapshot current = CreateCurrentSnapshot();
	undoStack_.push_back(std::move(current));
	ApplyEditorSnapshot(redoStack_.back());
	redoStack_.pop_back();
	hasUnsavedChanges_ = true;
}
void Hierarchy::ApplyEditorSnapshot(const EditorSnapshot& snapshot) {
	editorTransforms_ = snapshot.objectTransforms;
	editorMaterials_ = snapshot.objectMaterials;
	objectNames_ = snapshot.objectNames;
	primitiveEditorTransforms_ = snapshot.primitiveTransforms;
	primitiveEditorMaterials_ = snapshot.primitiveMaterials;
	primitiveNames_ = snapshot.primitiveNames;

	for (size_t i = 0; i < objects_.size(); ++i) {
		if (!objects_[i] || i >= editorTransforms_.size() || i >= editorMaterials_.size()) {
			continue;
		}
		EditorObject3d::ApplyEditorValues(objects_[i], editorTransforms_[i], editorMaterials_[i]);
	}

	for (size_t i = 0; i < primitives_.size(); ++i) {
		if (!primitives_[i] || primitives_[i] == selectionBoxPrimitive_.get() || i >= primitiveEditorTransforms_.size() || i >= primitiveEditorMaterials_.size()) {
			continue;
		}
		EditorPrimitive::ApplyEditorValues(primitives_[i], primitiveEditorTransforms_[i], primitiveEditorMaterials_[i]);
	}

	selectionBoxDirty_ = true;
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
		EditorObject3d::ApplyEditorValues(object, editorTransforms_[index], editorMaterials_[index]);
		return;
	}
	const size_t index = objects_.size();
	objects_.push_back(object);
	objectNames_.push_back("Object " + std::to_string(index));
	editorTransforms_.push_back(object->GetTransform());
	editorMaterials_.push_back(EditorObject3d::CaptureMaterial(object));
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
		EditorPrimitive::ApplyEditorValues(primitive, primitiveEditorTransforms_[index], primitiveEditorMaterials_[index]);
		return;
	}
	const size_t index = primitives_.size();
	primitives_.push_back(primitive);
	primitiveNames_.push_back("Primitive " + std::to_string(index));
	primitiveEditorTransforms_.push_back(primitive->GetTransform());
	primitiveEditorMaterials_.push_back(EditorPrimitive::CaptureMaterial(primitive));
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
void Hierarchy::RegisterCamera(Camera* camera) {
#ifdef USE_IMGUI
	editorCamera_.RegisterCamera(camera);
#else
	(void)camera;
#endif
}

void Hierarchy::UnregisterCamera(Camera* camera) {
#ifdef USE_IMGUI
	editorCamera_.UnregisterCamera(camera);
#else
	(void)camera;
#endif
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
	editorLight_.SaveToJson(root["lights"]);
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
		objectJson["editorId"] = object->GetEditorId();
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
		const InspectorMaterial& material = primitiveEditorMaterials_[i];
		nlohmann::json primitiveJson;
		primitiveJson["index"] = i;
		primitiveJson["name"] = primitiveNames_[i];
		primitiveJson["editorId"] = primitive->GetEditorId();
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

	editorAudio_.SaveToJson(root["audio"]);
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

	std::unordered_map<std::string, size_t> objectIdToIndex;
	for (size_t i = 0; i < objects_.size(); ++i) {
		if (!objects_[i]) {
			continue;
		}
		const std::string& editorId = objects_[i]->GetEditorId();
		if (!editorId.empty()) {
			objectIdToIndex.emplace(editorId, i);
		}
	}

	if (root.contains("objects") && root["objects"].is_array()) {
		for (const auto& objectJson : root["objects"]) {
			size_t index = std::numeric_limits<size_t>::max();
			if (objectJson.contains("editorId") && objectJson["editorId"].is_string()) {
				const std::string editorId = objectJson["editorId"].get<std::string>();
				const auto idIt = objectIdToIndex.find(editorId);
				if (idIt != objectIdToIndex.end()) {
					index = idIt->second;
				}
			}
			if (index == std::numeric_limits<size_t>::max()) {
				if (!objectJson.contains("index") || !objectJson["index"].is_number_unsigned()) {
					continue;
				}
				index = objectJson["index"].get<size_t>();
			}
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
			EditorObject3d::ApplyEditorValues(objects_[index], editorTransforms_[index], material);
		}
	}

	std::unordered_map<std::string, size_t> primitiveIdToIndex;
	for (size_t i = 0; i < primitives_.size(); ++i) {
		if (!primitives_[i] || primitives_[i] == selectionBoxPrimitive_.get()) {
			continue;
		}
		const std::string& editorId = primitives_[i]->GetEditorId();
		if (!editorId.empty()) {
			primitiveIdToIndex.emplace(editorId, i);
		}
	}

	if (root.contains("primitives") && root["primitives"].is_array()) {
		for (const auto& primitiveJson : root["primitives"]) {
			size_t index = std::numeric_limits<size_t>::max();
			if (primitiveJson.contains("editorId") && primitiveJson["editorId"].is_string()) {
				const std::string editorId = primitiveJson["editorId"].get<std::string>();
				const auto idIt = primitiveIdToIndex.find(editorId);
				if (idIt != primitiveIdToIndex.end()) {
					index = idIt->second;
				}
			}
			if (index == std::numeric_limits<size_t>::max()) {
				if (!primitiveJson.contains("index") || !primitiveJson["index"].is_number_unsigned()) {
					continue;
				}
				index = primitiveJson["index"].get<size_t>();
			}
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
			EditorPrimitive::ApplyEditorValues(primitives_[index], primitiveEditorTransforms_[index], material);
		}
	}

	if (root.contains("lights") && root["lights"].is_object()) {
		editorLight_.LoadFromJson(root["lights"]);
	}
	editorAudio_.LoadFromJson(root.value("audio", nlohmann::json::object()));
	loadedSnapshot_ = CreateCurrentSnapshot();
	hasLoadedSnapshot_ = true;
	loadedSnapshotFilePath_ = filePath;
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


void Hierarchy::SetPlayMode(bool isPlaying) {
	const bool wasPlaying = isPlaying_;
	isPlaying_ = isPlaying;
	if (!isPlaying_) {
		isPaused_ = false;
	}
	editorAudio_.OnPlayModeChanged(isPlaying_, wasPlaying);
	if (isPlaying_) {
		isPaused_ = false;
		editorCamera_.DeactivatePreview();
	}
}

void Hierarchy::DrawEditorGridLines() {
#ifdef USE_IMGUI
	DrawCameraBillboards();
	EditorGrid::DrawEditorGridLines(gridSettings_, editorGridPlane_);
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
void Hierarchy::DrawCameraBillboards() { editorCamera_.DrawCameraBillboards(isPlaying_); }
void Hierarchy::DrawCameraEditor() { editorCamera_.DrawCameraEditor(); }
void Hierarchy::DrawLightEditor() {
#ifdef USE_IMGUI
	if (editorLight_.DrawEditor(isPlaying_)) {
		hasUnsavedChanges_ = true;
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
			EditorObject3d::ApplyEditorValues(object, editorTransforms_[i], editorMaterials_[i]);
		}

		for (size_t i = 0; i < primitives_.size(); ++i) {
			Primitive* primitive = primitives_[i];
			if (!primitive) {
				continue;
			}
			EditorPrimitive::ApplyEditorValues(primitive, primitiveEditorTransforms_[i], primitiveEditorMaterials_[i]);
		}
	}
	const ImGuiViewport* viewport = ImGui::GetMainViewport();
	const float kTopToolbarHeight = 80.0f;
	const float kLeftPanelRatio = 0.22f;
	const float kRightPanelRatio = 0.24f;
	const float kPanelMinWidth = 260.0f;
	const float availableHeight = std::max(1.0f, viewport->WorkSize.y - kTopToolbarHeight);
	const float leftPanelWidth = std::max(kPanelMinWidth, viewport->WorkSize.x * kLeftPanelRatio);
	const float rightPanelWidth = std::max(kPanelMinWidth, viewport->WorkSize.x * kRightPanelRatio);


	ImGui::SetNextWindowPos(ImVec2(viewport->WorkPos.x, viewport->WorkPos.y), ImGuiCond_Always);
	ImGui::SetNextWindowSize(ImVec2(viewport->WorkSize.x, kTopToolbarHeight), ImGuiCond_Always);
	if (ImGui::Begin("Toolbar", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_MenuBar)) {
		ToolBar::Result toolbarResult = ToolBar::Draw(isPlaying_, isPaused_, hasUnsavedChanges_, !undoStack_.empty(), !redoStack_.empty(), showGridWindow_, gridSettings_);
		if (toolbarResult.saveRequested) {
			if (!isPlaying_) {
				const std::string saveFilePath = GetSceneScopedEditorFilePath("objectEditors.json");
				const bool saved = SaveObjectEditorsToJson(saveFilePath);
				if (saved) {
					hasUnsavedChanges_ = false;
				}
				saveStatusMessage_ = saved ? ("Saved: " + saveFilePath) : ("Save failed: " + saveFilePath);
			}
		}
		if (toolbarResult.spriteVisibilityChanged) {
			saveStatusMessage_ = SpriteCommon::GetInstance()->IsSpriteVisible() ? "View: show all sprites" : "View: hide all sprites";
		}
		if (toolbarResult.allResetRequested) {
			const bool reset = ResetToLoadedSnapshot();
			saveStatusMessage_ = reset ? "AllReset: restored loaded values" : "AllReset failed: no loaded data";
		}
		if (toolbarResult.undoRequested) {
			UndoEditorChange();
			saveStatusMessage_ = "Undo";
		}
		if (toolbarResult.redoRequested) {
			RedoEditorChange();
			saveStatusMessage_ = "Redo";
		}
		if (toolbarResult.pauseRequested && isPlaying_) {
			isPaused_ = true;
			saveStatusMessage_ = "Paused";
		}
		if (toolbarResult.resumeRequested && isPlaying_) {
			isPaused_ = false;
			saveStatusMessage_ = "Playing";
		}
		if (toolbarResult.playRequested) {
			if (hasUnsavedChanges_) {
				ImGui::OpenPopup("Unsaved Changes");
			} else {
				SceneManager::GetInstance()->RequestReinitializeCurrentScene();
				SetPlayMode(true);
				saveStatusMessage_ = "Playing";
			}
		}

		if (ImGui::BeginPopupModal("Unsaved Changes", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
			ImGui::TextUnformatted("There are unsaved changes. Do you want to continue playing?");
			ImGui::Separator();

			if (ImGui::Button("YES", ImVec2(100.0f, 0.0f))) {
				SceneManager::GetInstance()->RequestReinitializeCurrentScene();
				SetPlayMode(true);
				saveStatusMessage_ = "Playing (unsaved changes kept)";
				ImGui::CloseCurrentPopup();
			}
			ImGui::SameLine();
			if (ImGui::Button("NO", ImVec2(100.0f, 0.0f))) {
				saveStatusMessage_ = "Play canceled";
				ImGui::CloseCurrentPopup();
			}
			ImGui::SameLine();
			if (ImGui::Button("Save", ImVec2(100.0f, 0.0f))) {
				const std::string saveFilePath = GetSceneScopedEditorFilePath("objectEditors.json");
				const bool saved = SaveObjectEditorsToJson(saveFilePath);
				if (saved) {
					hasUnsavedChanges_ = false;
					SceneManager::GetInstance()->RequestReinitializeCurrentScene();
					SetPlayMode(true);
					saveStatusMessage_ = "Saved and Playing: " + saveFilePath;
					ImGui::CloseCurrentPopup();
				} else {
					saveStatusMessage_ = "Save failed: " + saveFilePath;
				}
			}

			ImGui::EndPopup();
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
	if (ImGui::Begin("Hierarchy", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_HorizontalScrollbar)) {
		ImGui::Text("Auto Object Editor");
		ImGui::Separator();
		ImGui::SeparatorText("Scene Switch");
		DrawSceneSelector();
		ImGui::SeparatorText("Light");
		DrawLightEditor();
		ImGui::SeparatorText("Camera");
		DrawCameraEditor();
		ImGui::SeparatorText("Audio");
		editorAudio_.DrawEditor(isPlaying_, hasUnsavedChanges_);
		ImGui::SeparatorText("Selection");
		DrawSelectionBoxEditor();
		ImGui::Separator();


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
	if (ImGui::Begin("Inspector", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_HorizontalScrollbar)) {
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
						EditorPrimitive::ApplyEditorValues(primitive, transform, material);
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
						EditorObject3d::ApplyEditorValues(object, transform, material);
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