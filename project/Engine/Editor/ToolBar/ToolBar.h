class ToolBar final {
public:
	struct Result final {
		bool saveRequested = false;
		bool gridRequested = false;
		bool allResetRequested = false;
		bool undoRequested = false;
		bool redoRequested = false;
		bool playRequested = false;
		bool stopRequested = false;
		bool spriteVisibilityChanged = false;
	};

	static Result Draw(bool isPlaying, bool hasUnsavedChanges, bool canUndo, bool canRedo);
};