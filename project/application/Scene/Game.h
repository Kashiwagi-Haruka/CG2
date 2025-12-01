#pragma once
#include "FrameWork.h"
#include"GameBase.h"
#include "D3DResourceLeakChecker.h"
#include "Scene/GameScene.h"
#include "Scene/ResultScene.h"
#include "Scene/TitleScene.h"
class Game :public FrameWork{
	
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

	

	public:

	void Initialize() override;
	void Update() override;
	void Draw() override;
	void Finalize() override;

	




};
