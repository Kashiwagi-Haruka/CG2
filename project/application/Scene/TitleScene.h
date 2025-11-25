#pragma once
#include "Sprite.h"
class GameBase;
class TitleScene {
	
	bool isSceneEnd_;

	



public: 
	~TitleScene();
	void Initialize();
	void Update();
	void Draw();
	bool GetIsSceneEnd() { return isSceneEnd_; }
};
