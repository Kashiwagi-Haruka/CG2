#include "ToolBar.h"

#ifdef USE_IMGUI
#include "Engine/Texture/TextureManager.h"
#include "Sprite/SpriteCommon.h"
#include "externals/imgui/imgui.h"
#endif

ToolBar::Result ToolBar::Draw(bool isPlaying, bool isPaused, bool hasUnsavedChanges, bool canUndo, bool canRedo, bool& showGridMenu, EditorGrid::Settings& gridSettings) {
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
		if (ImGui::BeginMenu("View")) {
			ImGui::MenuItem("Grid", nullptr, &showGridMenu);
			if (showGridMenu) {
				ImGui::Separator();
				if (ImGui::BeginMenu("Grid Settings")) {
					EditorGrid::DrawSettingsEditor(gridSettings);
					ImGui::EndMenu();
				}
			}
			const bool isSpriteVisible = SpriteCommon::GetInstance()->IsSpriteVisible();
			if (ImGui::MenuItem("All Sprites", nullptr, isSpriteVisible)) {
				SpriteCommon::GetInstance()->SetSpriteVisible(!isSpriteVisible);
				result.spriteVisibilityChanged = true;
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
	constexpr const char* kPauseIconPath = "Resources/Editor/pause.png";
	constexpr const char* kStopIconPath = "Resources/Editor/stop.png";
	const ImVec2 iconButtonSize(24.0f, 24.0f);
	const auto playHandle = TextureManager::GetInstance()->GetSrvHandleGPU(kPlayIconPath);
	const auto pauseHandle = TextureManager::GetInstance()->GetSrvHandleGPU(kPauseIconPath);

	if (!isPlaying) {
		if (ImGui::ImageButton("PlayIconButton", ImTextureRef(reinterpret_cast<void*>(playHandle.ptr)), iconButtonSize)) {
			result.playRequested = true;
		}
		ImGui::SameLine();
		ImGui::BeginDisabled();
		ImGui::ImageButton("PauseIconButton", ImTextureRef(reinterpret_cast<void*>(pauseHandle.ptr)), iconButtonSize);
		ImGui::EndDisabled();
		if (hasUnsavedChanges) {
			ImGui::SameLine();
			ImGui::TextUnformatted("(Unsaved changes)");
		}
	} else {
		if (isPaused) {
			if (ImGui::ImageButton("PlayIconButton", ImTextureRef(reinterpret_cast<void*>(playHandle.ptr)), iconButtonSize)) {
				result.resumeRequested = true;
			}
		} else {
			ImGui::BeginDisabled();
			ImGui::ImageButton("PlayIconButton", ImTextureRef(reinterpret_cast<void*>(playHandle.ptr)), iconButtonSize);
			ImGui::EndDisabled();
		}
		ImGui::SameLine();
		if (!isPaused && ImGui::ImageButton("PauseIconButton", ImTextureRef(reinterpret_cast<void*>(pauseHandle.ptr)), iconButtonSize)) {
			result.pauseRequested = true;
		}
		if (isPaused) {
			ImGui::BeginDisabled();
			ImGui::ImageButton("PauseIconButton", ImTextureRef(reinterpret_cast<void*>(pauseHandle.ptr)), iconButtonSize);
			ImGui::EndDisabled();
		}
		ImGui::SameLine();
		const auto stopHandle = TextureManager::GetInstance()->GetSrvHandleGPU(kStopIconPath);
		if (ImGui::ImageButton("StopIconButton", ImTextureRef(reinterpret_cast<void*>(stopHandle.ptr)), iconButtonSize)) {
			result.stopRequested = true;
		}
		ImGui::SameLine();
		ImGui::TextUnformatted(isPaused ? "Paused" : "Playing");
	}
#else
	(void)isPlaying;
	(void)isPaused;
	(void)hasUnsavedChanges;
	(void)canUndo;
	(void)canRedo;
	(void)showGridMenu;
	(void)gridSettings;
#endif
	return result;
}