#pragma once
#include "GameObject/Portal/Portal.h"
#include "GameObject/Portal/PortalParticle.h"
#include "Transform.h"
#include "Vector3.h"

class Player;
class PlayerCamera;
class WhiteBoard;
class WhiteBoardManager;

class PortalManager {
public:
	PortalManager(Vector3* pos, WhiteBoardManager* whiteBoardManager);
	~PortalManager();
	void Initialize();
    void SpawnFirstPortal();
	void Update();
	void WarpPlayer(Player* player);
	void Draw(bool isShadow, bool drawPortal, bool drawParticle);

	void SetCamera(Camera* camera);
	void SetPlayerCamera(PlayerCamera* playerCamera);
	/// @brief 作成できるポータル地点との当たり判定を作成する
	void CheckCollision();
	std::vector<std::unique_ptr<Portal>>& GetPortals() { return portals_; };

	static bool GetCanMakePortal() { return canMakePortal_; };

private:
	void UpdatePortal();

	// ポータルの作成
	void SpawnPortal(WhiteBoard* board);
	void DrawPortal();

	static bool canMakePortal_;

	WhiteBoardManager* whiteBoardManager_ = nullptr;
	WhiteBoard* pendingWhiteBoard_ = nullptr;

	PlayerCamera* playerCamera_ = nullptr;
	Vector3* playerPos_ = nullptr;
	// 初回のワープ地点
	Transform firstWarpPosTransform_ = {0.0f};

	bool isPendingPortalSpawn_ = false;
	std::vector<std::unique_ptr<Portal>> portals_;
	std::unique_ptr<PortalParticle> portalParticle_ = nullptr;

	const float kWarpTime_ = 2.0f;
	float warpCoolTimer_ = kWarpTime_;
};