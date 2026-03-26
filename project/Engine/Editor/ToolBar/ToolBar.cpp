#include "ToolBar.h"

#ifdef USE_IMGUI
#include "Engine/Texture/TextureManager.h"
#include "externals/imgui/imgui.h"
#endif

ToolBar::Result ToolBar::Draw(bool isPlaying, bool hasUnsavedChanges, bool canUndo, bool canRedo) {
	Result result{};
#ifdef USE_IMGUI
	if (ImGui::BeginMenuBar()) {
		if (ImGui::BeginMenu("File")) {
			if (!isPlaying && ImGui::MenuItem("Save")) {
				result.saveRequested = true;
			}
			if (!isPlaying && ImGui::MenuItem("AllReset")) {
				result.allResetRequested = true;
			}
			ImGui::EndMenu();
		}
		ImGui::EndMenuBar();
	}

	if (!canUndo) {
		ImGui::BeginDisabled();
	}
	if (ImGui::Button("Undo")) {
		result.undoRequested = true;
	}
	if (!canUndo) {
		ImGui::EndDisabled();
	}

	ImGui::SameLine();
	if (!canRedo) {
		ImGui::BeginDisabled();
	}
	if (ImGui::Button("Redo")) {
		result.redoRequested = true;
	}
	if (!canRedo) {
		ImGui::EndDisabled();
	}

	ImGui::SameLine();
	ImGui::TextUnformatted("Mode");
	ImGui::SameLine();

	constexpr const char* kPlayIconPath = "Resources/Editor/play.png";
	constexpr const char* kStopIconPath = "Resources/Editor/stop.png";
	const ImVec2 iconButtonSize(24.0f, 24.0f);

	if (!isPlaying) {
		const auto playHandle = TextureManager::GetInstance()->GetSrvHandleGPU(kPlayIconPath);
		if (ImGui::ImageButton("PlayIconButton", ImTextureRef(reinterpret_cast<void*>(playHandle.ptr)), iconButtonSize)) {
			result.playRequested = true;
		}
		if (hasUnsavedChanges) {
			ImGui::SameLine();
			ImGui::TextUnformatted("(Unsaved changes)");
		}
	} else {
		const auto stopHandle = TextureManager::GetInstance()->GetSrvHandleGPU(kStopIconPath);
		if (ImGui::ImageButton("StopIconButton", ImTextureRef(reinterpret_cast<void*>(stopHandle.ptr)), iconButtonSize)) {
			result.stopRequested = true;
		}
		ImGui::SameLine();
		ImGui::TextUnformatted("Playing");
	}
#else
	(void)isPlaying;
	(void)hasUnsavedChanges;
	(void)canUndo;
	(void)canRedo;
#endif
	return result;
}