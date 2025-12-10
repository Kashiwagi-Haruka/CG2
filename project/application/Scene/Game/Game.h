#pragma once
#include "FrameWork.h"
#include"GameBase.h"
#include "D3DResourceLeakChecker.h"

class SceneFactory;
class BaseScene;
class Game :public FrameWork{
	


	D3DResourceLeakChecker* d3dResourceLeakChecker = nullptr;

	SceneFactory* sceneFactory_ = nullptr;


	public:

	void Initialize() override;
	void Update() override;
	void Draw() override;
	void Finalize() override;

	




};
