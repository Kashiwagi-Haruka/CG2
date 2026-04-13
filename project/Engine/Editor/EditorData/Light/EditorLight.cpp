#include "EditorLight.h"

#include "Object3d/Object3dCommon.h"

#include <string>

#ifdef USE_IMGUI
#include "externals/imgui/imgui.h"
#endif

namespace {
void LoadColor(const nlohmann::json& json, Vector4& outColor) {
	if (json.is_array() && json.size() == 4) {
		outColor = {json[0].get<float>(), json[1].get<float>(), json[2].get<float>(), json[3].get<float>()};
	}
}

void LoadVector3(const nlohmann::json& json, Vector3& outVector) {
	if (json.is_array() && json.size() == 3) {
		outVector = {json[0].get<float>(), json[1].get<float>(), json[2].get<float>()};
	}
}
} // namespace

void EditorLight::Reset() {
	state_ = State{};
	Object3dCommon::GetInstance()->SetEditorLightOverride(false);
}

void EditorLight::ApplyOverride() const {
	Object3dCommon::GetInstance()->SetEditorLightOverride(state_.overrideSceneLights);
	PushToObject3dCommon();
}

bool EditorLight::DrawEditor(bool isPlaying) {
#ifdef USE_IMGUI
	bool overrideChanged = ImGui::Checkbox("Use Editor Lights", &state_.overrideSceneLights);
	if (overrideChanged) {
		Object3dCommon::GetInstance()->SetEditorLightOverride(state_.overrideSceneLights);
	}

	bool lightChanged = false;
	if (ImGui::TreeNode("Directional Light")) {
		if (!isPlaying) {
			lightChanged |= ImGui::ColorEdit4("Dir Color", &state_.directionalLight.color.x);
			lightChanged |= ImGui::DragFloat3("Dir Direction", &state_.directionalLight.direction.x, 0.01f, -1.0f, 1.0f);
			lightChanged |= ImGui::DragFloat("Dir Intensity", &state_.directionalLight.intensity, 0.01f, 0.0f, 10.0f);
		}
		ImGui::TreePop();
	}

	if (ImGui::TreeNode("Point Lights")) {
		int pointCount = static_cast<int>(state_.pointLights.size());
		if (!isPlaying && ImGui::SliderInt("Point Count", &pointCount, 0, static_cast<int>(kMaxPointLights))) {
			state_.pointLights.resize(static_cast<size_t>(pointCount));
			lightChanged = true;
		}
		for (size_t i = 0; i < state_.pointLights.size(); ++i) {
			PointCommonLight& point = state_.pointLights[i];
			const std::string label = "Point " + std::to_string(i);
			if (ImGui::TreeNode((label + "##point").c_str())) {
				if (!isPlaying) {
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
		int spotCount = static_cast<int>(state_.spotLights.size());
		if (!isPlaying && ImGui::SliderInt("Spot Count", &spotCount, 0, static_cast<int>(kMaxSpotLights))) {
			state_.spotLights.resize(static_cast<size_t>(spotCount));
			lightChanged = true;
		}
		for (size_t i = 0; i < state_.spotLights.size(); ++i) {
			SpotCommonLight& spot = state_.spotLights[i];
			if (ImGui::TreeNode(("Spot " + std::to_string(i) + "##spot").c_str())) {
				if (!isPlaying) {
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
		int areaCount = static_cast<int>(state_.areaLights.size());
		if (!isPlaying && ImGui::SliderInt("Area Count", &areaCount, 0, static_cast<int>(kMaxAreaLights))) {
			state_.areaLights.resize(static_cast<size_t>(areaCount));
			lightChanged = true;
		}
		for (size_t i = 0; i < state_.areaLights.size(); ++i) {
			AreaCommonLight& area = state_.areaLights[i];
			if (ImGui::TreeNode(("Area " + std::to_string(i) + "##area").c_str())) {
				if (!isPlaying) {
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

	if (lightChanged || state_.overrideSceneLights) {
		PushToObject3dCommon();
	}
	return overrideChanged || lightChanged;
#else
	(void)isPlaying;
	return false;
#endif
}

void EditorLight::SaveToJson(nlohmann::json& lightsJson) const {
	lightsJson = {
	    {"overrideSceneLights", state_.overrideSceneLights},
	    {"directional",
	     {
	         {"color", {state_.directionalLight.color.x, state_.directionalLight.color.y, state_.directionalLight.color.z, state_.directionalLight.color.w}},
	         {"direction", {state_.directionalLight.direction.x, state_.directionalLight.direction.y, state_.directionalLight.direction.z}},
	         {"intensity", state_.directionalLight.intensity},
	     }	                                            },
	    {"point",               nlohmann::json::array()   },
	    {"spot",                nlohmann::json::array()   },
	    {"area",                nlohmann::json::array()   },
	};

	for (const PointCommonLight& point : state_.pointLights) {
		lightsJson["point"].push_back({
		    {"color",     {point.color.x, point.color.y, point.color.z, point.color.w}},
		    {"position",  {point.position.x, point.position.y, point.position.z}      },
		    {"intensity", point.intensity		                                     },
		    {"radius",    point.radius		                                        },
		    {"decay",     point.decay		                                         },
		});
	}
	for (const SpotCommonLight& spot : state_.spotLights) {
		lightsJson["spot"].push_back({
		    {"color",           {spot.color.x, spot.color.y, spot.color.z, spot.color.w}},
		    {"position",        {spot.position.x, spot.position.y, spot.position.z}     },
		    {"direction",       {spot.direction.x, spot.direction.y, spot.direction.z}  },
		    {"intensity",       spot.intensity		                                  },
		    {"distance",        spot.distance		                                   },
		    {"decay",           spot.decay		                                      },
		    {"cosAngle",        spot.cosAngle		                                   },
		    {"cosFalloffStart", spot.cosFalloffStart                                    },
		});
	}
	for (const AreaCommonLight& area : state_.areaLights) {
		lightsJson["area"].push_back({
		    {"color",     {area.color.x, area.color.y, area.color.z, area.color.w}},
		    {"position",  {area.position.x, area.position.y, area.position.z}     },
		    {"normal",    {area.normal.x, area.normal.y, area.normal.z}           },
		    {"intensity", area.intensity		                                  },
		    {"width",     area.width		                                      },
		    {"height",    area.height		                                     },
		    {"radius",    area.radius		                                     },
		    {"decay",     area.decay		                                      },
		});
	}
}

void EditorLight::LoadFromJson(const nlohmann::json& lightsJson) {
	Reset();
	if (!lightsJson.is_object()) {
		return;
	}
	if (lightsJson.contains("overrideSceneLights") && lightsJson["overrideSceneLights"].is_boolean()) {
		state_.overrideSceneLights = lightsJson["overrideSceneLights"].get<bool>();
	}
	if (lightsJson.contains("directional") && lightsJson["directional"].is_object()) {
		const auto& directionalJson = lightsJson["directional"];
		if (directionalJson.contains("color")) {
			LoadColor(directionalJson["color"], state_.directionalLight.color);
		}
		if (directionalJson.contains("direction")) {
			LoadVector3(directionalJson["direction"], state_.directionalLight.direction);
		}
		if (directionalJson.contains("intensity") && directionalJson["intensity"].is_number()) {
			state_.directionalLight.intensity = directionalJson["intensity"].get<float>();
		}
	}
	if (lightsJson.contains("point") && lightsJson["point"].is_array()) {
		for (const auto& pointJson : lightsJson["point"]) {
			if (!pointJson.is_object())
				continue;
			PointCommonLight point{};
			if (pointJson.contains("color"))
				LoadColor(pointJson["color"], point.color);
			if (pointJson.contains("position"))
				LoadVector3(pointJson["position"], point.position);
			if (pointJson.contains("intensity") && pointJson["intensity"].is_number())
				point.intensity = pointJson["intensity"].get<float>();
			if (pointJson.contains("radius") && pointJson["radius"].is_number())
				point.radius = pointJson["radius"].get<float>();
			if (pointJson.contains("decay") && pointJson["decay"].is_number())
				point.decay = pointJson["decay"].get<float>();
			state_.pointLights.push_back(point);
			if (state_.pointLights.size() >= kMaxPointLights)
				break;
		}
	}
	if (lightsJson.contains("spot") && lightsJson["spot"].is_array()) {
		for (const auto& spotJson : lightsJson["spot"]) {
			if (!spotJson.is_object())
				continue;
			SpotCommonLight spot{};
			if (spotJson.contains("color"))
				LoadColor(spotJson["color"], spot.color);
			if (spotJson.contains("position"))
				LoadVector3(spotJson["position"], spot.position);
			if (spotJson.contains("direction"))
				LoadVector3(spotJson["direction"], spot.direction);
			if (spotJson.contains("intensity") && spotJson["intensity"].is_number())
				spot.intensity = spotJson["intensity"].get<float>();
			if (spotJson.contains("distance") && spotJson["distance"].is_number())
				spot.distance = spotJson["distance"].get<float>();
			if (spotJson.contains("decay") && spotJson["decay"].is_number())
				spot.decay = spotJson["decay"].get<float>();
			if (spotJson.contains("cosAngle") && spotJson["cosAngle"].is_number())
				spot.cosAngle = spotJson["cosAngle"].get<float>();
			if (spotJson.contains("cosFalloffStart") && spotJson["cosFalloffStart"].is_number())
				spot.cosFalloffStart = spotJson["cosFalloffStart"].get<float>();
			state_.spotLights.push_back(spot);
			if (state_.spotLights.size() >= kMaxSpotLights)
				break;
		}
	}
	if (lightsJson.contains("area") && lightsJson["area"].is_array()) {
		for (const auto& areaJson : lightsJson["area"]) {
			if (!areaJson.is_object())
				continue;
			AreaCommonLight area{};
			if (areaJson.contains("color"))
				LoadColor(areaJson["color"], area.color);
			if (areaJson.contains("position"))
				LoadVector3(areaJson["position"], area.position);
			if (areaJson.contains("normal"))
				LoadVector3(areaJson["normal"], area.normal);
			if (areaJson.contains("intensity") && areaJson["intensity"].is_number())
				area.intensity = areaJson["intensity"].get<float>();
			if (areaJson.contains("width") && areaJson["width"].is_number())
				area.width = areaJson["width"].get<float>();
			if (areaJson.contains("height") && areaJson["height"].is_number())
				area.height = areaJson["height"].get<float>();
			if (areaJson.contains("radius") && areaJson["radius"].is_number())
				area.radius = areaJson["radius"].get<float>();
			if (areaJson.contains("decay") && areaJson["decay"].is_number())
				area.decay = areaJson["decay"].get<float>();
			state_.areaLights.push_back(area);
			if (state_.areaLights.size() >= kMaxAreaLights)
				break;
		}
	}
	ApplyOverride();
}

void EditorLight::PushToObject3dCommon() const {
	Object3dCommon::GetInstance()->SetEditorLights(
	    state_.directionalLight, state_.pointLights.empty() ? nullptr : state_.pointLights.data(), static_cast<uint32_t>(state_.pointLights.size()),
	    state_.spotLights.empty() ? nullptr : state_.spotLights.data(), static_cast<uint32_t>(state_.spotLights.size()), state_.areaLights.empty() ? nullptr : state_.areaLights.data(),
	    static_cast<uint32_t>(state_.areaLights.size()));
}