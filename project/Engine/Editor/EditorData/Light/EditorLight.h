#pragma once

#include "Light/CommonLight/AreaCommonLight.h"
#include "Light/CommonLight/DirectionalCommonLight.h"
#include "Light/CommonLight/PointCommonLight.h"
#include "Light/CommonLight/SpotCommonLight.h"
#include <nlohmann/json.hpp>

#include <vector>

class EditorLight {
public:
	struct State {
		bool overrideSceneLights = false;
		DirectionalCommonLight directionalLight = {
		    {1.0f, 1.0f, 1.0f, 1.0f},
            {0.0f, -1.0f, 0.0f},
            1.0f, 1, {0.0f, 0.0f, 0.0f}
        };
		std::vector<PointCommonLight> pointLights;
		std::vector<SpotCommonLight> spotLights;
		std::vector<AreaCommonLight> areaLights;
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