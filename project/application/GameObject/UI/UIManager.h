#pragma once
#include <memory>

#include "GameObject/UI/RaySprite.h"
#include "GameObject/UI/TabKey.h"
#include "KeyIcon.h"
#include "LightIcon.h"
#include "Menu/Menu.h"
#include "Mission.h"
#include "Text/TextUIManager/TextUIManager.h"

class UIManager {
private:
	static bool isPause_;
	std::unique_ptr<TextUIManager> textUIManager_ = nullptr;
	std::unique_ptr<Menu> menu_ = nullptr;
	// raySprite
	std::unique_ptr<RaySprite> raySprite_ = nullptr;
	std::unique_ptr<TabKey> tabKey_ = nullptr;

	std::unique_ptr<KeyIcon> keyIcon_ = nullptr;
	std::unique_ptr<LightIcon> lightIcon_ = nullptr;
	std::unique_ptr<Mission> mission_ = nullptr;

public:
	static bool GetIsPause() { return isPause_; }
	static void SetIsPause(const bool isPause) { isPause_ = isPause; }
	static void TogglePause();
	UIManager();
	void Initialize();
	void Update();
	void CloseOptionANdPrepareResume();
	void CursorShowAndMove();
	void CursorHideAndStop();
	void Draw();
};
