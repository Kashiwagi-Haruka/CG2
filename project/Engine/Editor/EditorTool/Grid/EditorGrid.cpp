#define NOMINMAX
#include "EditorGrid.h"

#include "Function.h"
#include "Object3d/Object3dCommon.h"
#include "Primitive/Primitive.h"

#include <algorithm>
#include <cstdlib>

#ifdef USE_IMGUI
#include "externals/imgui/imgui.h"
#endif

std::vector<EditorGridLine> EditorGrid::CreateLines(int halfLineCount, float spacing, float lineWidth) {
	if (halfLineCount < 0) {
		halfLineCount = 0;
	}
	if (spacing <= 0.0f) {
		spacing = 1.0f;
	}

	const int lineCount = halfLineCount * 2 + 1;
	std::vector<EditorGridLine> lines;
	lines.reserve(static_cast<size_t>(lineCount) * 2u);

	const float min = -static_cast<float>(halfLineCount) * spacing;
	const float max = static_cast<float>(halfLineCount) * spacing;

	for (int i = -halfLineCount; i <= halfLineCount; ++i) {
		const float offset = static_cast<float>(i) * spacing;
		const Vector4 color = GetLineColor(i);

		lines.push_back({
		    {min, 0.0f, offset},
		    {max, 0.0f, offset},
		    color,
		    lineWidth,
		});
		lines.push_back({
		    {offset, 0.0f, min},
		    {offset, 0.0f, max},
		    color,
		    lineWidth,
		});
	}

	return lines;
}

void EditorGrid::DrawSettingsEditor(Settings& settings) {
#ifdef USE_IMGUI
	ImGui::Checkbox("Enable Grid Snap", &settings.enableSnap);
	if (ImGui::DragFloat("Grid Snap Spacing", &settings.snapSpacing, 0.05f, 0.1f, 100.0f, "%.2f")) {
		settings.dirty = true;
	}
	if (ImGui::Checkbox("Draw Editor Grid Lines", &settings.showLines)) {
		settings.dirty = true;
	}
	if (ImGui::DragInt("Grid Half Line Count", &settings.halfLineCount, 1.0f, 1, 200)) {
		settings.dirty = true;
	}
	if (ImGui::DragFloat("Grid Y", &settings.y, 0.01f, -100.0f, 100.0f, "%.2f")) {
		settings.dirty = true;
	}
#else
	(void)settings;
#endif
	ClampSettings(settings);
}

void EditorGrid::ClampSettings(Settings& settings) {
	settings.snapSpacing = std::max(settings.snapSpacing, 0.1f);
	settings.halfLineCount = std::max(settings.halfLineCount, 1);
}

void EditorGrid::DrawEditorGridLines(Settings& settings, std::unique_ptr<Primitive>& gridPlane) {
	if (!settings.showLines) {
		return;
	}

	if (settings.dirty || !gridPlane) {
		if (!gridPlane) {
			gridPlane = std::make_unique<Primitive>();
			gridPlane->SetEditorRegistrationEnabled(false);
			gridPlane->Initialize(Primitive::Plane);
			gridPlane->SetCamera(Object3dCommon::GetInstance()->GetDefaultCamera());
			gridPlane->SetEnableLighting(false);
		}

		const float extent = static_cast<float>(settings.halfLineCount) * settings.snapSpacing;
		Transform gridTransform{};
		gridTransform.scale = {extent * 2.0f, extent * 2.0f, 1.0f};
		gridTransform.rotate = {Function::kPi * 0.5f, 0.0f, 0.0f};
		gridTransform.translate = {0.0f, settings.y, 0.0f};
		gridPlane->SetTransform(gridTransform);
		gridPlane->SetColor({1.0f, 1.0f, 1.0f, 1.0f});
		gridPlane->SetDistortionFalloff(settings.snapSpacing);
		gridPlane->SetDistortionStrength(static_cast<float>(settings.halfLineCount));
		gridPlane->SetEnvironmentCoefficient(settings.snapSpacing * 0.025f);
		settings.dirty = false;
	}

	if (!gridPlane) {
		return;
	}

	gridPlane->SetCamera(Object3dCommon::GetInstance()->GetDefaultCamera());
	Object3dCommon::GetInstance()->DrawCommonEditorGrid();
	gridPlane->Update();
	gridPlane->Draw();
}

Vector4 EditorGrid::GetLineColor(int index) {
	if (index == 0) {
		return {1.0f, 1.0f, 0.0f, 1.0f}; // origin: yellow
	}

	const int oneDigit = std::abs(index) % 10;

	if (oneDigit == 0) {
		return {1.0f, 0.0f, 0.0f, 1.0f}; // ones digit 0: red
	}
	if (oneDigit == 5) {
		return {0.0f, 1.0f, 0.0f, 1.0f}; // ones digit 5: green
	}
	return {1.0f, 1.0f, 1.0f, 1.0f}; // others: white
}