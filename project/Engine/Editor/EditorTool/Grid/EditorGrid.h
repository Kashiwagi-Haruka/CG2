#pragma once

#include "Engine/math/Vector3.h"
#include "Engine/math/Vector4.h"

#include <memory>
#include <vector>

class Primitive;

struct EditorGridLine final {
	Vector3 start;
	Vector3 end;
	Vector4 color;
	float width;
};

class EditorGrid final {
public:
	struct Settings final {
		bool enableSnap = true;
		float snapSpacing = 1.0f;
		bool showLines = true;
		int halfLineCount = 50;
		float y = 0.0f;
		bool dirty = true;
	};

	static std::vector<EditorGridLine> CreateLines(int halfLineCount = 50, float spacing = 1.0f, float lineWidth = 1.0f);
	static void DrawSettingsEditor(Settings& settings);
	static void ClampSettings(Settings& settings);
	static void DrawEditorGridLines(Settings& settings, std::unique_ptr<Primitive>& gridPlane);

private:
	static Vector4 GetLineColor(int index);
};