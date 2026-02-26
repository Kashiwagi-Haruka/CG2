#include "ToolBar.h"

#ifdef USE_IMGUI
#include "externals/imgui/imgui.h"
#endif

ToolBar::Result ToolBar::Draw(bool isPlaying, bool hasUnsavedChanges) {
	Result result{};
#ifdef USE_IMGUI
	ImGui::TextUnformatted("Mode");
	ImGui::SameLine();
	if (!isPlaying) {
		if (ImGui::Button("Play")) {
			result.playRequested = true;
		}
		if (hasUnsavedChanges) {
			ImGui::SameLine();
			ImGui::TextUnformatted("(Unsaved changes)");
		}
	} else {
		if (ImGui::Button("Stop")) {
			result.stopRequested = true;
		}
		ImGui::SameLine();
		ImGui::TextUnformatted("Playing");
	}
#else
	(void)isPlaying;
	(void)hasUnsavedChanges;
#endif
	return result;
}
