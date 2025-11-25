#pragma once
#include <imgui.h>
class GameBase;
class ResultScene {

	bool isSceneEnd_;


	public:
	void Initialize();
	void Update(bool isGameClear, bool isGameOver);
	void Draw();
	bool GetIsSceneEnd() { return isSceneEnd_; }
};
