#pragma once
#include "BaseScene.h"
class SceneManager {

	static SceneManager* instance_;

	BaseScene* scene_ = nullptr;
	BaseScene* nextscene_ = nullptr;

	public:

		
		void SetNextScene(BaseScene* nextScene) { nextscene_ = nextScene; };
	    void Update();
	    void Draw();
	    void Finalize();
	    static SceneManager* GetInstance();
};
