#pragma once
#include <imgui.h>
class GameBase;
class ResultScene {

	bool isSceneEnd_;


	public:
	void Initialize(GameBase* gameBase);
	void Update(bool isGameClear, bool isGameOver,GameBase* gameBase);
	void Draw();
	bool GetIsSceneEnd() { return isSceneEnd_; }
};
