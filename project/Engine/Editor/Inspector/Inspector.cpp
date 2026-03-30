#include "Inspector.h"

#ifdef USE_IMGUI
#include "externals/imgui/imgui.h"
#endif

#include <algorithm>
#include <array>
#ifdef USE_IMGUI
namespace {
bool DrawAxisValue(const char* axisLabel, const ImVec4& axisColor, const std::string& id, float& value) {
	bool changed = false;
	ImGui::PushStyleColor(ImGuiCol_Text, axisColor);
	ImGui::TextUnformatted(axisLabel);
	ImGui::PopStyleColor();
	ImGui::SameLine();
	ImGui::SetNextItemWidth(80.0f);
	changed |= ImGui::DragFloat(id.c_str(), &value, 0.01f);
	return changed;
}

bool DrawTransformAxisRow(const char* label, const std::string& idSuffix, Vector3& value) {
	constexpr float kAxisStartX = 100.0f;
	bool changed = false;
	ImGui::TextUnformatted(label);
	ImGui::SameLine(kAxisStartX);
	changed |= DrawAxisValue("X:", ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "##x_" + idSuffix, value.x);
	ImGui::SameLine();
	changed |= DrawAxisValue("Y:", ImVec4(0.3f, 1.0f, 0.3f, 1.0f), "##y_" + idSuffix, value.y);
	ImGui::SameLine();
	changed |= DrawAxisValue("Z:", ImVec4(0.3f, 0.5f, 1.0f, 1.0f), "##z_" + idSuffix, value.z);
	return changed;
}
} // namespace
#endif
bool Inspector::DrawObjectInspector(
    size_t index, std::string& objectName, Transform& transform, InspectorMaterial& material, bool isPlaying, bool& transformChanged, bool& materialChanged, bool& nameChanged) {
#ifdef USE_IMGUI
	ImGui::Text("Object3d #%zu", index);
	if (isPlaying) {
		ImGui::TextUnformatted("Playing... inspector values are locked");
		return false;
	}
	std::array<char, 128> nameBuffer{};
	const size_t copyLength = std::min(nameBuffer.size() - 1, objectName.size());
	std::copy_n(objectName.begin(), copyLength, nameBuffer.begin());
	if (ImGui::InputText(("Name##object_" + std::to_string(index)).c_str(), nameBuffer.data(), nameBuffer.size())) {
		objectName = nameBuffer.data();
		nameChanged = true;
	}
	transformChanged |= DrawTransformEditor("object_" + std::to_string(index), transform);
	materialChanged |= DrawMaterialEditor("object_" + std::to_string(index), material);
	return transformChanged || materialChanged || nameChanged;
#else
	(void)index;
	(void)objectName;
	(void)transform;
	(void)material;
	(void)isPlaying;
	(void)transformChanged;
	(void)materialChanged;
	(void)nameChanged;
	return false;
#endif
}

bool Inspector::DrawPrimitiveInspector(
    size_t index, std::string& primitiveName, Transform& transform, InspectorMaterial& material, bool isPlaying, bool& transformChanged, bool& materialChanged, bool& nameChanged) {
#ifdef USE_IMGUI
	ImGui::Text("Primitive #%zu", index);
	if (isPlaying) {
		ImGui::TextUnformatted("Playing... inspector values are locked");
		return false;
	}
	std::array<char, 128> nameBuffer{};
	const size_t copyLength = std::min(nameBuffer.size() - 1, primitiveName.size());
	std::copy_n(primitiveName.begin(), copyLength, nameBuffer.begin());
	if (ImGui::InputText(("Name##primitive_" + std::to_string(index)).c_str(), nameBuffer.data(), nameBuffer.size())) {
		primitiveName = nameBuffer.data();
		nameChanged = true;
	}
	transformChanged |= DrawTransformEditor("primitive_" + std::to_string(index), transform);
	materialChanged |= DrawMaterialEditor("primitive_" + std::to_string(index), material);
	return transformChanged || materialChanged || nameChanged;
#else
	(void)index;
	(void)primitiveName;
	(void)transform;
	(void)material;
	(void)isPlaying;
	(void)transformChanged;
	(void)materialChanged;
	(void)nameChanged;
	return false;
#endif
}

bool Inspector::DrawTransformEditor(const std::string& idSuffix, Transform& transform) {
#ifdef USE_IMGUI
	bool changed = false;
	changed |= DrawTransformAxisRow("Scale", "scale_" + idSuffix, transform.scale);
	changed |= DrawTransformAxisRow("Rotate", "rotate_" + idSuffix, transform.rotate);
	changed |= DrawTransformAxisRow("Translate", "translate_" + idSuffix, transform.translate);
	return changed;
#else
	(void)idSuffix;
	(void)transform;
	return false;
#endif
}

bool Inspector::DrawMaterialEditor(const std::string& idSuffix, InspectorMaterial& material) {
#ifdef USE_IMGUI
	bool changed = false;
	ImGui::SeparatorText("Material");
	changed |= ImGui::ColorEdit4(("Color##" + idSuffix).c_str(), &material.color.x);
	changed |= ImGui::Checkbox(("Enable Lighting##" + idSuffix).c_str(), &material.enableLighting);
	changed |= ImGui::DragFloat(("Shininess##" + idSuffix).c_str(), &material.shininess, 0.1f, 0.0f, 256.0f);
	changed |= ImGui::DragFloat(("Environment##" + idSuffix).c_str(), &material.environmentCoefficient, 0.01f, 0.0f, 1.0f);
	changed |= ImGui::Checkbox(("Grayscale##" + idSuffix).c_str(), &material.grayscaleEnabled);
	changed |= ImGui::Checkbox(("Sepia##" + idSuffix).c_str(), &material.sepiaEnabled);
	changed |= ImGui::DragFloat(("Distortion Strength##" + idSuffix).c_str(), &material.distortionStrength, 0.01f, -10.0f, 10.0f);
	changed |= ImGui::DragFloat(("Distortion Falloff##" + idSuffix).c_str(), &material.distortionFalloff, 0.01f, 0.0f, 10.0f);
	ImGui::SeparatorText("UV Distortion");
	changed |= ImGui::DragFloat3(("UV Scale##" + idSuffix).c_str(), &material.uvScale.x, 0.01f);
	changed |= ImGui::DragFloat3(("UV Rotate##" + idSuffix).c_str(), &material.uvRotate.x, 0.01f);
	changed |= ImGui::DragFloat3(("UV Translate##" + idSuffix).c_str(), &material.uvTranslate.x, 0.01f);
	changed |= ImGui::DragFloat2(("UV Anchor##" + idSuffix).c_str(), &material.uvAnchor.x, 0.01f);
	return changed;
#else
	(void)idSuffix;
	(void)material;
	return false;
#endif
}