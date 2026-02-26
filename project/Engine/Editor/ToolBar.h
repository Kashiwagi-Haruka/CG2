#pragma once

class ToolBar final {
public:
	struct Result final {
		bool playRequested = false;
		bool stopRequested = false;
	};

	static Result Draw(bool isPlaying, bool hasUnsavedChanges);
};