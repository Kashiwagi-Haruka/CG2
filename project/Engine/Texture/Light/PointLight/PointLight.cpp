#include "PointLight.h"

#include <algorithm>

PointLight::PointLight() = default;

void PointLight::AddPointLight(std::string name) {
	auto it = std::find_if(pointLights_.begin(), pointLights_.end(), [&](const Data& data) { return data.name == name; });
	if (it == pointLights_.end()) {
		pointLights_.push_back({{}, std::move(name)});
	}
}

void PointLight::RemovePointLight(const std::string& name) {
	pointLights_.erase(std::remove_if(pointLights_.begin(), pointLights_.end(), [&](const Data& data) { return data.name == name; }), pointLights_.end());
}

void PointLight::SetLightColor(const std::string& name, const Vector4& color) {
	for (auto& data : pointLights_)
		if (data.name == name) {
			data.light.color = color;
			break;
		}
}
void PointLight::SetLightPosition(const std::string& name, const Vector3& position) {
	for (auto& data : pointLights_)
		if (data.name == name) {
			data.light.position = position;
			break;
		}
}
void PointLight::SetLightIntensity(const std::string& name, float intensity) {
	for (auto& data : pointLights_)
		if (data.name == name) {
			data.light.intensity = intensity;
			break;
		}
}
void PointLight::SetLightRadius(const std::string& name, float radius) {
	for (auto& data : pointLights_)
		if (data.name == name) {
			data.light.radius = radius;
			break;
		}
}
void PointLight::SetLightDecay(const std::string& name, float decay) {
	for (auto& data : pointLights_)
		if (data.name == name) {
			data.light.decay = decay;
			break;
		}
}
void PointLight::SetLightProperties(const std::string& name, const Vector4& color, const Vector3& position, float intensity, float radius, float decay) {
	for (auto& data : pointLights_)
		if (data.name == name) {
			data.light.color = color;
			data.light.position = position;
			data.light.intensity = intensity;
			data.light.radius = radius;
			data.light.decay = decay;
			break;
		}
}

Vector4 PointLight::GetLightColor(const std::string& name) const {
	for (const auto& data : pointLights_)
		if (data.name == name)
			return data.light.color;
	return {1.0f, 1.0f, 1.0f, 1.0f};
}
Vector3 PointLight::GetLightPosition(const std::string& name) const {
	for (const auto& data : pointLights_)
		if (data.name == name)
			return data.light.position;
	return {0.0f, 0.0f, 0.0f};
}
float PointLight::GetLightIntensity(const std::string& name) const {
	for (const auto& data : pointLights_)
		if (data.name == name)
			return data.light.intensity;
	return 0.0f;
}
float PointLight::GetLightRadius(const std::string& name) const {
	for (const auto& data : pointLights_)
		if (data.name == name)
			return data.light.radius;
	return 0.0f;
}
float PointLight::GetLightDecay(const std::string& name) const {
	for (const auto& data : pointLights_)
		if (data.name == name)
			return data.light.decay;
	return 0.0f;
}

void PointLight::ClearLights() { pointLights_.clear(); }