#include "EditorAsset.h"
#include "Engine/Texture/TextureManager.h"
#include <algorithm>
#include <imgui.h>

void EditorAsset::EditorDraw() {
#ifdef USE_IMGUI

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
		auto categoryToLabel = [](AssetCategory category) -> const char* {
			switch (category) {
			case AssetCategory::Audio:
				return "Audio";
			case AssetCategory::Camera:
				return "Camera";
			case AssetCategory::Sprite:
				return "Sprite";
			case AssetCategory::Primitive:
				return "Primitive";
			case AssetCategory::Object3d:
				return "Object3d";
			case AssetCategory::Material:
				return "Material";
			case AssetCategory::None:
			default:
				return "";
			}
		};

		struct AssetButtonDef {
			const char* id;
			const char* label;
			AssetCategory category;
		};
		constexpr AssetButtonDef kAssetButtons[] = {
		    {"##AudioAssetButton",     "Audio",     AssetCategory::Audio    },
            {"##CameraAssetButton",    "Camera",    AssetCategory::Camera   },
		    {"##SpriteAssetButton",    "Sprite",    AssetCategory::Sprite   },
            {"##PrimitiveAssetButton", "Primitive", AssetCategory::Primitive},
		    {"##Object3dAssetButton",  "Object3d",  AssetCategory::Object3d },
            {"##MaterialAssetButton",  "Material",  AssetCategory::Material },
		};

		ImGui::TextUnformatted("Asset >");
		if (selectedCategory_ != AssetCategory::None) {
			ImGui::SameLine(0.0f, 8.0f);
			ImGui::Text("%s >", categoryToLabel(selectedCategory_));
		}
		ImGui::Separator();

		constexpr const char* kFileIconPath = "Resources/Editor/File.png";
		const ImVec2 buttonSize(96.0f, 112.0f);
		const ImVec2 iconSize(48.0f, 48.0f);
		const float iconTopOffset = 16.0f;
		const float textBottomOffset = 14.0f;
		const auto fileIconHandle = TextureManager::GetInstance()->GetSrvHandleGPU(kFileIconPath);
		const float buttonSpacing = 12.0f;
		const float windowContentMaxX = ImGui::GetWindowPos().x + ImGui::GetWindowContentRegionMax().x;

		ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(0, 0, 0, 0));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, IM_COL32(255, 255, 255, 32));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, IM_COL32(255, 255, 255, 48));
		ImGui::PushStyleColor(ImGuiCol_Border, IM_COL32(0, 0, 0, 0));
		ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);

		for (const auto& assetButton : kAssetButtons) {
			const ImVec2 buttonPos = ImGui::GetCursorScreenPos();
			if (ImGui::Button(assetButton.id, buttonSize)) {
				selectedCategory_ = assetButton.category;
			}

			ImDrawList* drawList = ImGui::GetWindowDrawList();
			const ImVec2 iconPos(buttonPos.x + (buttonSize.x - iconSize.x) * 0.5f, buttonPos.y + iconTopOffset);
			drawList->AddImage(ImTextureRef(reinterpret_cast<void*>(fileIconHandle.ptr)), iconPos, ImVec2(iconPos.x + iconSize.x, iconPos.y + iconSize.y));

			const ImVec2 labelSize = ImGui::CalcTextSize(assetButton.label);
			const ImVec2 labelPos(buttonPos.x + (buttonSize.x - labelSize.x) * 0.5f, buttonPos.y + buttonSize.y - textBottomOffset - labelSize.y);
			drawList->AddText(labelPos, IM_COL32(255, 255, 255, 255), assetButton.label);

			const float nextButtonRightEdge = buttonPos.x + buttonSize.x + buttonSpacing + buttonSize.x;
			if (nextButtonRightEdge <= windowContentMaxX) {
				ImGui::SameLine(0.0f, buttonSpacing);
			}
		}
		ImGui::PopStyleVar();
		ImGui::PopStyleColor(4);
	}
	ImGui::End();
#endif // USE_IMGUI
}