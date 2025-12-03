#pragma once
#include "FrameWork.h"
#include"GameBase.h"
#include "D3DResourceLeakChecker.h"

class BaseScene;
class Game :public FrameWork{
	


	D3DResourceLeakChecker* d3dResourceLeakChecker = nullptr;




	public:

	void Initialize() override;
	void Update() override;
	void Draw() override;
	void Finalize() override;

	




};
