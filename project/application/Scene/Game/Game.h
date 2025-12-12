#pragma once
#include "FrameWork.h"
#include"GameBase.h"
#include "D3DResourceLeakChecker.h"
#include <memory>
#include "Scene/SceneFactory/SceneFactory.h"
class BaseScene;
class Game :public FrameWork{
	


	D3DResourceLeakChecker d3dResourceLeakChecker;

	std::unique_ptr<SceneFactory> sceneFactory_ = nullptr;


	public:

	void Initialize() override;
	void Update() override;
	void Draw() override;
	void Finalize() override;
	~Game()=default;
	




};
