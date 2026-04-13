#include "EditorAsset.h"
#include <algorithm>
#include <imgui.h>

void EditorAsset::EditorDraw() {
	const ImGuiViewport* viewport = ImGui::GetMainViewport();
	const float kTopToolbarHeight = 45.0f;
	const float kLeftPanelRatio = 0.22f;
	const float kRightPanelRatio = 0.24f;
	const float kPanelMinWidth = 260.0f;
	const float kGameAspect = 16.0f / 9.0f;
	const float leftPanelWidth = (std::max)(kPanelMinWidth, viewport->WorkSize.x * kLeftPanelRatio);
	const float rightPanelWidth = (std::max)(kPanelMinWidth, viewport->WorkSize.x * kRightPanelRatio);
	const float availableWidth = (std::max)(1.0f, viewport->WorkSize.x - leftPanelWidth - rightPanelWidth);
	const float availableHeight = (std::max)(1.0f, viewport->WorkSize.y - kTopToolbarHeight);

	float gameHeight = availableWidth / kGameAspect;
	if (gameHeight > availableHeight) {
		gameHeight = availableHeight;
	}

	const float assetWindowPosX = viewport->WorkPos.x + leftPanelWidth;
	const float assetWindowPosY = viewport->WorkPos.y + kTopToolbarHeight + gameHeight;
	const float assetWindowWidth = availableWidth;
	const float assetWindowHeight = (std::max)(1.0f, availableHeight - gameHeight);

	ImGui::SetNextWindowPos(ImVec2(assetWindowPosX, assetWindowPosY), ImGuiCond_Always);
	ImGui::SetNextWindowSize(ImVec2(assetWindowWidth, assetWindowHeight), ImGuiCond_Always);
	if (ImGui::Begin("Asset", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_HorizontalScrollbar)) {
		ImGui::TextUnformatted("Editor Asset Window");
	}
	ImGui::End();
}