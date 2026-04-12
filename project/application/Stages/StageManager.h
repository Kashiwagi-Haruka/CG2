#pragma once
#include <string>
class Player;
class StageManager {


	Player* player_ = nullptr;

	public:
	void CreateStage(const std::string& sceneName);
	void InitializeStage();
	void UpdateLight();
	void UpdatePlayerDamage();
	void UpdatePostEffect();
	void UpdateGameObject();
	void UpdatePortal();
	void CheckCollision();
	void DrawModel();
	void DrawDamageOverlay();

};
