#include "SpotLight.h"

#include <algorithm>
#include <cmath>

SpotLight::SpotLight() = default;

void SpotLight::AddSpotLight(std::string name) {
	auto it = std::find_if(spotLights_.begin(), spotLights_.end(), [&](const Data& data) { return data.name == name; });
	if (it == spotLights_.end()) {
		spotLights_.push_back({{}, std::move(name)});
	}
}
void SpotLight::RemoveSpotLight(std::string name) {
	spotLights_.erase(std::remove_if(spotLights_.begin(), spotLights_.end(), [&](const Data& data) { return data.name == name; }), spotLights_.end());
}

void SpotLight::SetSpotLightColor(const std::string& name, const Vector4& color) {
	for (auto& d : spotLights_)
		if (d.name == name) {
			d.light.color = color;
			break;
		}
}
void SpotLight::SetSpotLightPosition(const std::string& name, const Vector3& position) {
	for (auto& d : spotLights_)
		if (d.name == name) {
			d.light.position = position;
			break;
		}
}
void SpotLight::SetSpotLightIntensity(const std::string& name, float intensity) {
	for (auto& d : spotLights_)
		if (d.name == name) {
			d.light.intensity = intensity;
			break;
		}
}
void SpotLight::SetSpotLightDirection(const std::string& name, const Vector3& direction) {
	for (auto& d : spotLights_)
		if (d.name == name) {
			d.light.direction = direction;
			break;
		}
}
void SpotLight::SetSpotLightDistance(const std::string& name, float distance) {
	for (auto& d : spotLights_)
		if (d.name == name) {
			d.light.distance = distance;
			break;
		}
}
void SpotLight::SetSpotLightDecay(const std::string& name, float decay) {
	for (auto& d : spotLights_)
		if (d.name == name) {
			d.light.decay = decay;
			break;
		}
}
void SpotLight::SetSpotLightAngle(const std::string& name, float angle) {
	for (auto& d : spotLights_)
		if (d.name == name) {
			d.light.cosAngle = std::cos(angle);
			break;
		}
}
void SpotLight::SetSpotLightFalloffStartAngle(const std::string& name, float falloffStartAngle) {
	for (auto& d : spotLights_)
		if (d.name == name) {
			d.light.cosFalloffStart = std::cos(falloffStartAngle);
			break;
		}
}
void SpotLight::SetSpotLightName(const std::string& oldName, const std::string& newName) {
	for (auto& d : spotLights_)
		if (d.name == oldName) {
			d.name = newName;
			break;
		}
}

void SpotLight::SetSpotLightProperties(
    const std::string& name, const Vector4& color, const Vector3& position, float intensity, const Vector3& direction, float distance, float decay, float angle, float falloffStartAngle) {
	for (auto& d : spotLights_)
		if (d.name == name) {
			d.light.color = color;
			d.light.position = position;
			d.light.intensity = intensity;
			d.light.direction = direction;
			d.light.distance = distance;
			d.light.decay = decay;
			d.light.cosAngle = std::cos(angle);
			d.light.cosFalloffStart = std::cos(falloffStartAngle);
			break;
		}
}

Vector4 SpotLight::GetSpotLightColor(const std::string& name) const {
	for (const auto& d : spotLights_)
		if (d.name == name)
			return d.light.color;
	return {1, 1, 1, 1};
}
Vector3 SpotLight::GetSpotLightPosition(const std::string& name) const {
	for (const auto& d : spotLights_)
		if (d.name == name)
			return d.light.position;
	return {0, 0, 0};
}
float SpotLight::GetSpotLightIntensity(const std::string& name) const {
	for (const auto& d : spotLights_)
		if (d.name == name)
			return d.light.intensity;
	return 0.0f;
}
Vector3 SpotLight::GetSpotLightDirection(const std::string& name) const {
	for (const auto& d : spotLights_)
		if (d.name == name)
			return d.light.direction;
	return {0, -1, 0};
}
float SpotLight::GetSpotLightDistance(const std::string& name) const {
	for (const auto& d : spotLights_)
		if (d.name == name)
			return d.light.distance;
	return 0.0f;
}
float SpotLight::GetSpotLightDecay(const std::string& name) const {
	for (const auto& d : spotLights_)
		if (d.name == name)
			return d.light.decay;
	return 0.0f;
}
float SpotLight::GetSpotLightAngle(const std::string& name) const {
	for (const auto& d : spotLights_)
		if (d.name == name)
			return std::acos(d.light.cosAngle);
	return 0.0f;
}
float SpotLight::GetSpotLightFalloffStartAngle(const std::string& name) const {
	for (const auto& d : spotLights_)
		if (d.name == name)
			return std::acos(d.light.cosFalloffStart);
	return 0.0f;
}

void SpotLight::ClearSpotLights() { spotLights_.clear(); }