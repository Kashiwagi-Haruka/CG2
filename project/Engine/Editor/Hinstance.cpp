#define NOMINMAX
#include "Hinstance.h"
#include "EditorGrid.h"
#include "Engine/BaseScene/SceneManager.h"
#include "Engine/Loadfile/JSON/JsonManager.h"
#include "Function.h"
#include "Object3d/Object3d.h"
#include "Object3d/Object3dCommon.h"
#include "Primitive/Primitive.h"
#ifdef USE_IMGUI
#include "externals/imgui/imgui.h"
#endif

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
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
	editorLightState_.overrideSceneLights = false;
	editorLightState_.directionalLight = {
	    {1.0f, 1.0f, 1.0f, 1.0f},
        {0.0f, -1.0f, 0.0f},
        1.0f
    };
	editorLightState_.pointLights.clear();
	editorLightState_.spotLights.clear();
	editorLightState_.areaLights.clear();
	Object3dCommon::GetInstance()->SetEditorLightOverride(false);
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

void Hinstance::DrawGridEditor() {
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
void Hinstance::SetPlayMode(bool isPlaying) { isPlaying_ = isPlaying; }

void Hinstance::DrawEditorGridLines() {
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
#endif
}

void Hinstance::DrawLightEditor() {
#ifdef USE_IMGUI
	bool overrideChanged = ImGui::Checkbox("Use Editor Lights", &editorLightState_.overrideSceneLights);
	if (overrideChanged) {
		Object3dCommon::GetInstance()->SetEditorLightOverride(editorLightState_.overrideSceneLights);
		hasUnsavedChanges_ = true;
	}

	bool lightChanged = false;
	if (ImGui::TreeNode("Directional Light")) {
		if (!isPlaying_) {
			lightChanged |= ImGui::ColorEdit4("Dir Color", &editorLightState_.directionalLight.color.x);
			lightChanged |= ImGui::DragFloat3("Dir Direction", &editorLightState_.directionalLight.direction.x, 0.01f, -1.0f, 1.0f);
			lightChanged |= ImGui::DragFloat("Dir Intensity", &editorLightState_.directionalLight.intensity, 0.01f, 0.0f, 10.0f);
		}
		ImGui::TreePop();
	}

	if (ImGui::TreeNode("Point Lights")) {
		int pointCount = static_cast<int>(editorLightState_.pointLights.size());
		if (!isPlaying_ && ImGui::SliderInt("Point Count", &pointCount, 0, static_cast<int>(kMaxPointLights))) {
			editorLightState_.pointLights.resize(static_cast<size_t>(pointCount));
			lightChanged = true;
		}
		for (size_t i = 0; i < editorLightState_.pointLights.size(); ++i) {
			PointLight& point = editorLightState_.pointLights[i];
			const std::string label = "Point " + std::to_string(i);
			if (ImGui::TreeNode((label + "##point").c_str())) {
				if (!isPlaying_) {
					lightChanged |= ImGui::ColorEdit4(("Color##point_" + std::to_string(i)).c_str(), &point.color.x);
					lightChanged |= ImGui::DragFloat3(("Position##point_" + std::to_string(i)).c_str(), &point.position.x, 0.05f);
					lightChanged |= ImGui::DragFloat(("Intensity##point_" + std::to_string(i)).c_str(), &point.intensity, 0.01f, 0.0f, 10.0f);
					lightChanged |= ImGui::DragFloat(("Radius##point_" + std::to_string(i)).c_str(), &point.radius, 0.05f, 0.0f, 500.0f);
					lightChanged |= ImGui::DragFloat(("Decay##point_" + std::to_string(i)).c_str(), &point.decay, 0.01f, 0.0f, 10.0f);
				}
				ImGui::TreePop();
			}
		}
		ImGui::TreePop();
	}

	if (ImGui::TreeNode("Spot Lights")) {
		int spotCount = static_cast<int>(editorLightState_.spotLights.size());
		if (!isPlaying_ && ImGui::SliderInt("Spot Count", &spotCount, 0, static_cast<int>(kMaxSpotLights))) {
			editorLightState_.spotLights.resize(static_cast<size_t>(spotCount));
			lightChanged = true;
		}
		for (size_t i = 0; i < editorLightState_.spotLights.size(); ++i) {
			SpotLight& spot = editorLightState_.spotLights[i];
			if (ImGui::TreeNode(("Spot " + std::to_string(i) + "##spot").c_str())) {
				if (!isPlaying_) {
					lightChanged |= ImGui::ColorEdit4(("Color##spot_" + std::to_string(i)).c_str(), &spot.color.x);
					lightChanged |= ImGui::DragFloat3(("Position##spot_" + std::to_string(i)).c_str(), &spot.position.x, 0.05f);
					lightChanged |= ImGui::DragFloat3(("Direction##spot_" + std::to_string(i)).c_str(), &spot.direction.x, 0.01f, -1.0f, 1.0f);
					lightChanged |= ImGui::DragFloat(("Intensity##spot_" + std::to_string(i)).c_str(), &spot.intensity, 0.01f, 0.0f, 10.0f);
					lightChanged |= ImGui::DragFloat(("Distance##spot_" + std::to_string(i)).c_str(), &spot.distance, 0.05f, 0.0f, 500.0f);
					lightChanged |= ImGui::DragFloat(("Decay##spot_" + std::to_string(i)).c_str(), &spot.decay, 0.01f, 0.0f, 10.0f);
					lightChanged |= ImGui::DragFloat(("Cos Angle##spot_" + std::to_string(i)).c_str(), &spot.cosAngle, 0.001f, -1.0f, 1.0f);
					lightChanged |= ImGui::DragFloat(("Cos Falloff##spot_" + std::to_string(i)).c_str(), &spot.cosFalloffStart, 0.001f, -1.0f, 1.0f);
				}
				ImGui::TreePop();
			}
		}
		ImGui::TreePop();
	}

	if (ImGui::TreeNode("Area Lights")) {
		int areaCount = static_cast<int>(editorLightState_.areaLights.size());
		if (!isPlaying_ && ImGui::SliderInt("Area Count", &areaCount, 0, static_cast<int>(kMaxAreaLights))) {
			editorLightState_.areaLights.resize(static_cast<size_t>(areaCount));
			lightChanged = true;
		}
		for (size_t i = 0; i < editorLightState_.areaLights.size(); ++i) {
			AreaLight& area = editorLightState_.areaLights[i];
			if (ImGui::TreeNode(("Area " + std::to_string(i) + "##area").c_str())) {
				if (!isPlaying_) {
					lightChanged |= ImGui::ColorEdit4(("Color##area_" + std::to_string(i)).c_str(), &area.color.x);
					lightChanged |= ImGui::DragFloat3(("Position##area_" + std::to_string(i)).c_str(), &area.position.x, 0.05f);
					lightChanged |= ImGui::DragFloat3(("Normal##area_" + std::to_string(i)).c_str(), &area.normal.x, 0.01f, -1.0f, 1.0f);
					lightChanged |= ImGui::DragFloat(("Intensity##area_" + std::to_string(i)).c_str(), &area.intensity, 0.01f, 0.0f, 10.0f);
					lightChanged |= ImGui::DragFloat(("Width##area_" + std::to_string(i)).c_str(), &area.width, 0.05f, 0.0f, 500.0f);
					lightChanged |= ImGui::DragFloat(("Height##area_" + std::to_string(i)).c_str(), &area.height, 0.05f, 0.0f, 500.0f);
					lightChanged |= ImGui::DragFloat(("Radius##area_" + std::to_string(i)).c_str(), &area.radius, 0.05f, 0.0f, 500.0f);
					lightChanged |= ImGui::DragFloat(("Decay##area_" + std::to_string(i)).c_str(), &area.decay, 0.01f, 0.0f, 10.0f);
				}
				ImGui::TreePop();
			}
		}
		ImGui::TreePop();
	}

	if (lightChanged || editorLightState_.overrideSceneLights) {
		hasUnsavedChanges_ = hasUnsavedChanges_ || lightChanged;
		Object3dCommon::GetInstance()->SetEditorLights(
		    editorLightState_.directionalLight, editorLightState_.pointLights.empty() ? nullptr : editorLightState_.pointLights.data(), static_cast<uint32_t>(editorLightState_.pointLights.size()),
		    editorLightState_.spotLights.empty() ? nullptr : editorLightState_.spotLights.data(), static_cast<uint32_t>(editorLightState_.spotLights.size()),
		    editorLightState_.areaLights.empty() ? nullptr : editorLightState_.areaLights.data(), static_cast<uint32_t>(editorLightState_.areaLights.size()));
	}
#endif
}
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
	ImGui::SeparatorText("Light");
	DrawLightEditor();
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