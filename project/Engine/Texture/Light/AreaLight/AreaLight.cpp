#include "AreaLight.h"

#include <algorithm>

AreaLight::AreaLight() = default;

void AreaLight::AddAreaLight(std::string name) {
	auto it = std::find_if(areaLights_.begin(), areaLights_.end(), [&](const Data& data) { return data.name == name; });
	if (it == areaLights_.end()) {
		areaLights_.push_back({{}, std::move(name)});
	}
}
void AreaLight::RemoveAreaLight(std::string name) {
	areaLights_.erase(std::remove_if(areaLights_.begin(), areaLights_.end(), [&](const Data& data) { return data.name == name; }), areaLights_.end());
}

void AreaLight::SetAreaLightColor(const std::string& name, const Vector4& color) {
	for (auto& d : areaLights_)
		if (d.name == name) {
			d.light.color = color;
			break;
		}
}
void AreaLight::SetAreaLightPosition(const std::string& name, const Vector3& position) {
	for (auto& d : areaLights_)
		if (d.name == name) {
			d.light.position = position;
			break;
		}
}
void AreaLight::SetAreaLightIntensity(const std::string& name, float intensity) {
	for (auto& d : areaLights_)
		if (d.name == name) {
			d.light.intensity = intensity;
			break;
		}
}
void AreaLight::SetAreaLightNormal(const std::string& name, const Vector3& normal) {
	for (auto& d : areaLights_)
		if (d.name == name) {
			d.light.normal = normal;
			break;
		}
}
void AreaLight::SetAreaLightSize(const std::string& name, float width, float height) {
	for (auto& d : areaLights_)
		if (d.name == name) {
			d.light.width = width;
			d.light.height = height;
			break;
		}
}
void AreaLight::SetAreaLightDistance(const std::string& name, float distance) {
	for (auto& d : areaLights_)
		if (d.name == name) {
			d.light.radius = distance;
			break;
		}
}
void AreaLight::SetAreaLightDecay(const std::string& name, float decay) {
	for (auto& d : areaLights_)
		if (d.name == name) {
			d.light.decay = decay;
			break;
		}
}

Vector4 AreaLight::GetAreaLightColor(const std::string& name) const {
	for (const auto& d : areaLights_)
		if (d.name == name)
			return d.light.color;
	return {1, 1, 1, 1};
}
Vector3 AreaLight::GetAreaLightPosition(const std::string& name) const {
	for (const auto& d : areaLights_)
		if (d.name == name)
			return d.light.position;
	return {0, 0, 0};
}
float AreaLight::GetAreaLightIntensity(const std::string& name) const {
	for (const auto& d : areaLights_)
		if (d.name == name)
			return d.light.intensity;
	return 0.0f;
}
Vector3 AreaLight::GetAreaLightNormal(const std::string& name) const {
	for (const auto& d : areaLights_)
		if (d.name == name)
			return d.light.normal;
	return {0, -1, 0};
}
Vector2 AreaLight::GetAreaLightSize(const std::string& name) const {
	for (const auto& d : areaLights_)
		if (d.name == name)
			return {d.light.width, d.light.height};
	return {0, 0};
}
float AreaLight::GetAreaLightDistance(const std::string& name) const {
	for (const auto& d : areaLights_)
		if (d.name == name)
			return d.light.radius;
	return 0.0f;
}
float AreaLight::GetAreaLightDecay(const std::string& name) const {
	for (const auto& d : areaLights_)
		if (d.name == name)
			return d.light.decay;
	return 0.0f;
}

void AreaLight::ClearAreaLights() { areaLights_.clear(); }