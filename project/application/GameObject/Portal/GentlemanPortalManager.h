#pragma once
#include "GameObject/Portal/Portal.h"
#include "Vector3.h"
#include"PortalManager.h"

class Player;
class PlayerCamera;
class GiantGentleMan;
class GentlemanPortalManager :public PortalManager {
public:
	GentlemanPortalManager(Vector3* pos);
	~GentlemanPortalManager();
	void SetGentleMan(GiantGentleMan* giantGentleMan);
	void Initialize()override;
	void Update()override;
	void WarpPlayer(Player* player)override;
	void Draw(bool isShadow, bool drawPortal, bool drawParticle)override;

	void SetCamera(Camera* camera)override;
	void SetPlayerCamera(PlayerCamera* playerCamera)override;
	/// @brief 作成できるポータル地点との当たり判定を作成する
	void CheckCollision()override;
	static bool GetCanMakePortal() { return canMakePortal_; };
private:
	void UpdatePortal();
	// ポータルの作成
	void SpawnPortal();
	void DrawPortal();

	GiantGentleMan* giantGentleman_ = nullptr;
	static bool canMakePortal_;
};