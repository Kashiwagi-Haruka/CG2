#pragma once
#include "GameObject/Portal/Portal.h"
#include "GameObject/Portal/PortalParticle.h"
#include "GameObject/WhiteBoard/WhiteBoard.h"
#include "Vector3.h"
#include <array>

class TimeCardWatch;
class PlayerCamera;
class PortalManager {
public:
	PortalManager(Vector3* pos);
	void Initialize();
	void UpdateWhiteBoard();
	void UpdatePortal();

	void ShadowDraw();
	void ObjDraw(bool drawParticle = true);
	void SetPlayerCamera(PlayerCamera* camera);
	/// @brief 作成できるポータル地点との当たり判定を作成する
	/// @param timeCardWatch 携帯打刻機
	/// @param camera かめら
	/// @param warpPos ワープ地点の設定をする
	void CheckCollision(TimeCardWatch* timeCardWatch, const Vector3& warpPos);
	std::vector<std::unique_ptr<Portal>>& GetPortals() { return portals_; };
	std::vector<std::unique_ptr<WhiteBoard>>& GetWhiteBoards() { return whiteBoards_; }

private:
	void SpawnPortal(WhiteBoard* board, const Vector3& warpPos);
	void DrawWhiteBoard();
	std::vector<std::unique_ptr<WhiteBoard>> whiteBoards_;
	std::vector<std::unique_ptr<Portal>> portals_;
	std::unique_ptr<PortalParticle> portalParticle_ = nullptr;
	PlayerCamera* playerCamera_ = nullptr;
	Vector3* playerPos_ = nullptr;
	WhiteBoard* whiteBoard_ = nullptr;
	WhiteBoard* pendingWhiteBoard_ = nullptr;
	Vector3 pendingWarpPos_{};
	uint32_t preCollision_;
	bool isPendingPortalSpawn_ = false;
};