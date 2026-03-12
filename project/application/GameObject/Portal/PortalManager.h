#pragma once
#include "GameObject/Portal/Portal.h"
#include "GameObject/Portal/PortalParticle.h"
#include "GameObject/WhiteBoard/WhiteBoard.h"
#include "Vector3.h"
#include"Transform.h"
#include <array>
class Player;
class TimeCardWatch;
class PlayerCamera;
class PortalManager {
public:
	PortalManager(Vector3* pos);
	void Initialize();
	void Update();
	void Draw(bool isShadow, bool drawPortal, bool drawParticle);
	void SetCamera(Camera* camera);
	void SetPlayerCamera(PlayerCamera* playerCamera);

	/// @brief 作成できるポータル地点との当たり判定を作成する
	/// @param timeCardWatch 携帯打刻機
	/// @param camera かめら
	/// @param warpPos ワープ地点の設定をする
	void CheckCollision(TimeCardWatch* timeCardWatch);
	std::vector<std::unique_ptr<Portal>>& GetPortals() { return portals_; };
	std::vector<std::unique_ptr<WhiteBoard>>& GetWhiteBoards() { return whiteBoards_; }
	void WarpPlayer(Player* player);
private:
	void UpdateWhiteBoard();
	void UpdatePortal();

	//ポータルの作成
	void SpawnPortal(WhiteBoard* board);
	void DrawWhiteBoard();
	void DrawPortal();

	std::vector<WhiteBoard*> preWhiteBoards_;
	std::vector<std::unique_ptr<WhiteBoard>> whiteBoards_;
	WhiteBoard* pendingWhiteBoard_ = nullptr;

	PlayerCamera* playerCamera_ = nullptr;
	Vector3* playerPos_ = nullptr;
	//初回のワープ地点
	Transform firstWarpPosTransform_ = { 0.0f };

	bool isPendingPortalSpawn_ = false;
	std::vector<std::unique_ptr<Portal>> portals_;
	std::unique_ptr<PortalParticle> portalParticle_ = nullptr;

	const float kWarpTime_ = 2.0f;
	float warpCoolTimer_ = kWarpTime_;

};