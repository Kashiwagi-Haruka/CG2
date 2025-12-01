#pragma once
#include"GameBase.h"
#include "D3DResourceLeakChecker.h"
#include "Scene/GameScene.h"
#include "Scene/ResultScene.h"
#include "Scene/TitleScene.h"
class Game {
	
	enum class SceneName {

		Title,
		Game,
		Result

	};

	D3DResourceLeakChecker* d3dResourceLeakChecker = nullptr;
	TitleScene* titleScene = nullptr;
	GameScene* gameScene = nullptr;
	ResultScene* resultScene = nullptr;
	SceneName scene;

	bool endReqest_ = false;

	public:

	void Initialize();
	void Update();
	void Draw();
	void Finalize();

	bool isEndRequest() { return endReqest_; }




};
