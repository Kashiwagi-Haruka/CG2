#define NOMINMAX
#include "Hinstance.h"
#include "Object3d/Object3d.h"
#ifdef USE_IMGUI
#include "externals/imgui/imgui.h"
#endif

#include <algorithm>
#include <string>

Hinstance* Hinstance::GetInstance() {
	static Hinstance instance;
	return &instance;
}

void Hinstance::RegisterObject3d(Object3d* object) {
	if (!object) {
		return;
	}
	if (std::find(objects_.begin(), objects_.end(), object) == objects_.end()) {
		objects_.push_back(object);
	}
}

void Hinstance::UnregisterObject3d(Object3d* object) {
	if (!object) {
		return;
	}
	auto it = std::remove(objects_.begin(), objects_.end(), object);
	objects_.erase(it, objects_.end());
}

bool Hinstance::HasRegisteredObjects() const { return !objects_.empty(); }

void Hinstance::DrawObjectEditors() {
#ifdef USE_IMGUI
	if (objects_.empty()) {
		return;
	}

	constexpr float kGameWidthRatio = 0.68f;
	constexpr float kEditorMinWidth = 280.0f;
	const ImGuiViewport* viewport = ImGui::GetMainViewport();
	const float editorWidthMax = viewport->WorkSize.x * (1.0f - kGameWidthRatio);
	const float editorWidth = std::max(kEditorMinWidth, std::min(editorWidthMax, viewport->WorkSize.x));
	const float editorPosX = viewport->WorkPos.x + viewport->WorkSize.x - editorWidth;
	const float editorPosY = viewport->WorkPos.y;

	ImGui::SetNextWindowPos(ImVec2(editorPosX, editorPosY), ImGuiCond_Always);

	if (!ImGui::Begin("Hinstance", nullptr, ImGuiWindowFlags_NoMove)) {
		ImGui::End();
		return;
	}

	ImGui::Text("Auto Object Editor");
	ImGui::Separator();
	for (size_t i = 0; i < objects_.size(); ++i) {
		Object3d* object = objects_[i];
		if (!object) {
			continue;
		}
		std::string nodeLabel = "Object " + std::to_string(i);
		if (ImGui::TreeNode((nodeLabel + "##node").c_str())) {
			Transform transform = object->GetTransform();
			bool changed = false;
			changed |= ImGui::DragFloat3(("Scale##" + std::to_string(i)).c_str(), &transform.scale.x, 0.01f);
			changed |= ImGui::DragFloat3(("Rotate##" + std::to_string(i)).c_str(), &transform.rotate.x, 0.01f);
			changed |= ImGui::DragFloat3(("Translate##" + std::to_string(i)).c_str(), &transform.translate.x, 0.01f);
			if (changed) {
				object->SetTransform(transform);
			}
			ImGui::TreePop();
		}
	}

	ImGui::End();
#endif
}