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
	PortalManager() = default;
	PortalManager(Vector3* pos);
	~PortalManager();
	void SetWhiteBoardManager(WhiteBoardManager* whiteBoardManager);
	virtual void Initialize();
	virtual void Update();
	virtual void WarpPlayer(Player* player);
	virtual void Draw(bool isShadow, bool drawPortal, bool drawParticle);

	virtual void SpawnFirstPortal();
	virtual void SetCamera(Camera* camera);
	virtual void SetPlayerCamera(PlayerCamera* playerCamera);
	/// @brief 作成できるポータル地点との当たり判定を作成する
	virtual void CheckCollision();
	virtual std::vector<std::unique_ptr<Portal>>& GetPortals() { return portals_; };
	static bool GetCanMakePortal() { return canMakePortal_; };

protected:
	// 初回のワープ地点
	Transform firstWarpPosTransform_ = { 0.0f };

	std::vector<std::unique_ptr<Portal>> portals_;
	PlayerCamera* playerCamera_ = nullptr;
	Vector3* playerPos_ = nullptr;
	bool isPendingPortalSpawn_ = false;
	std::unique_ptr<PortalParticle> portalParticle_ = nullptr;
	const float kWarpTime_ = 2.0f;
	float warpCoolTimer_ = kWarpTime_;
protected:
	virtual void UpdatePortal();
	virtual void DrawPortal();
private:

	// ポータルの作成
	void SpawnPortal(WhiteBoard* board);
	static bool canMakePortal_;
	WhiteBoardManager* whiteBoardManager_ = nullptr;
	WhiteBoard* pendingWhiteBoard_ = nullptr;

};