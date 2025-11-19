#pragma once
#include "Sprite.h"
class GameBase;
class TitleScene {
	
	bool isSceneEnd_;

	



public: 
	~TitleScene();
	void Initialize();
	void Update(GameBase*gameBase);
	void Draw();
	bool GetIsSceneEnd() { return isSceneEnd_; }
};
