#pragma once

#include "Engine/Editor/EditorTool/Grid/EditorGrid.h"

class ToolBar final {
public:
	struct Result final {
		bool saveRequested = false;
		bool allResetRequested = false;
		bool undoRequested = false;
		bool redoRequested = false;
		bool playRequested = false;
		bool pauseRequested = false;
		bool resumeRequested = false;
		bool stopRequested = false;
		bool spriteVisibilityChanged = false;
	};

	static Result Draw(bool isPlaying, bool isPaused, bool hasUnsavedChanges, bool canUndo, bool canRedo, bool& showGridMenu, EditorGrid::Settings& gridSettings);
};