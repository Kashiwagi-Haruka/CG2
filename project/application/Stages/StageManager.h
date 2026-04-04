#pragma once
#include <string>
class StageManager {


	



	public:

	void Finalize();
	void Update();
	void Draw();

	void ChangeStage(const std::string& stageName);

};
