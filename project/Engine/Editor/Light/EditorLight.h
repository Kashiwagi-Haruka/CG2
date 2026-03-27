#pragma once

#include "Light/AreaLight.h"
#include "Light/DirectionalLight.h"
#include "Light/PointLight.h"
#include "Light/SpotLight.h"
#include <nlohmann/json.hpp>

#include <vector>

class EditorLight {
public:
	struct State {
		bool overrideSceneLights = false;
		DirectionalLight directionalLight = {
		    {1.0f, 1.0f, 1.0f, 1.0f},
            {0.0f, -1.0f, 0.0f},
            1.0f, 1, {0.0f, 0.0f, 0.0f}
        };
		std::vector<PointLight> pointLights;
		std::vector<SpotLight> spotLights;
		std::vector<AreaLight> areaLights;
	};

	void Reset();
	State& GetState() { return state_; }
	const State& GetState() const { return state_; }
	void ApplyOverride() const;
	bool DrawEditor(bool isPlaying);
	void SaveToJson(nlohmann::json& lightsJson) const;
	void LoadFromJson(const nlohmann::json& lightsJson);

private:
	void PushToObject3dCommon() const;
	State state_{};
};