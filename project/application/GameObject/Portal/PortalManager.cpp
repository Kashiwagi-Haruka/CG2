#include "PortalManager.h"
#include "Object3d/Object3dCommon.h"
#include "GameObject/GameCamera/PlayerCamera/PlayerCamera.h"
#include "GameObject/KeyBindConfig.h"
#include "GameObject/TimeCard/TimeCardWatch.h"
#include "GameObject/WhiteBoard/WalkWhiteBoard.h"
#include "GameObject/YoshidaMath/YoshidaMath.h"
#include "Model/ModelManager.h"

namespace {
const constexpr uint32_t kMaxWhiteBoards = 6;
}

PortalManager::PortalManager(Vector3* pos) {
	playerPos_ = pos;
	ModelManager::GetInstance()->LoadGltfModel("Resources/TD3_3102/3d/whiteBoard", "whiteBoard");
	std::unique_ptr<WalkWhiteBoard> walkWhite = std::make_unique<WalkWhiteBoard>();
	WalkWhiteBoard::LoadAnimation("Resources/TD3_3102/3d/whiteBoard", "whiteBoard");
	walkWhite->SetModel("whiteBoard");
	walkWhite->SetTargetPosPtr(pos);
	whiteBoards_.push_back(std::move(walkWhite));

	for (int i = 0; i < kMaxWhiteBoards; ++i) {
		std::unique_ptr<WhiteBoard> white = std::make_unique<WhiteBoard>();
		white->SetModel("whiteBoard");
		whiteBoards_.push_back(std::move(white));
	}

	portalParticle_ = std::make_unique<PortalParticle>();
}

void PortalManager::Initialize() {
	for (auto& board : whiteBoards_) {
		board->Initialize();
	}

	portalParticle_->Initialize();
}

void PortalManager::UpdateWhiteBoard() {
	for (auto& board : whiteBoards_) {
		board->Update();
	}
}

void PortalManager::UpdatePortal() {

	if (isPendingPortalSpawn_ && portalParticle_) {
		portalParticle_->Update();
		if (portalParticle_->IsFinished() && pendingWhiteBoard_) {
			SpawnPortal(pendingWhiteBoard_, pendingWarpPos_);
			pendingWhiteBoard_ = nullptr;
			isPendingPortalSpawn_ = false;
		}
	}

	for (auto& portal : portals_) {
		portal->Update();
	}
};

void PortalManager::DrawWhiteBoard() {
	for (auto& board : whiteBoards_) {
		board->Draw();
	}
}

void PortalManager::ShadowDraw() {
	DrawWhiteBoard();

	for (auto& portal : portals_) {
		portal->SetCamera(playerCamera_->GetCamera());
		portal->UpdateCameraMatrices();
		portal->DrawRings();
		portal->DrawWarpPos();
	}
}

void PortalManager::ObjDraw(bool drawParticle) {

	DrawWhiteBoard();

	for (auto& portal : portals_) {
		portal->SetCamera(playerCamera_->GetCamera());
		portal->UpdateCameraMatrices();
		portal->DrawPortals();
		portal->DrawRings();
		portal->DrawWarpPos();
	}
	if (drawParticle && portalParticle_) {
		portalParticle_->Draw();
	}
	Object3dCommon::GetInstance()->DrawCommon();
}

void PortalManager::SetPlayerCamera(PlayerCamera* camera) {
	playerCamera_ = camera;
	for (auto& board : whiteBoards_) {
		board->SetCamera(camera->GetCamera());
	}

	if (portalParticle_) {
		portalParticle_->SetCamera(camera->GetCamera());
	}
}

void PortalManager::CheckCollision(TimeCardWatch* timeCardWatch, const Vector3& warpPos) {
	if (isPendingPortalSpawn_) {
		return;
	}

	// whiteBoardとrayの当たり判定
	for (auto& board : whiteBoards_) {
		if (timeCardWatch->OnCollisionObjOfMakePortal(playerCamera_->GetRay(), board->GetAABB(), board->GetTransform())) {

			if (PlayerCommand::GetInstance()->Shot()) {
				// 前回のポータルを削除
				portals_.clear();

				if (whiteBoard_) {
					whiteBoard_->SetCollisionAttribute(preCollision_);
				}

				// ショットしたらポータル作る
				whiteBoard_ = board.get();
				preCollision_ = whiteBoard_->GetCollisionAttribute();
				whiteBoard_->SetCollisionAttribute(kCollisionNone);

				pendingWhiteBoard_ = whiteBoard_;
				pendingWarpPos_ = warpPos;
				isPendingPortalSpawn_ = true;
				if (portalParticle_) {
					portalParticle_->Start(*playerPos_, whiteBoard_->GetTransform().translate);
				}
			}
			break;
		};
	}
}

void PortalManager::SpawnPortal(WhiteBoard* board, const Vector3& warpPos) {
	std::unique_ptr portal = std::make_unique<Portal>();

	portal->Initialize();

	Camera* camera = playerCamera_->GetCamera();
	portal->SetCamera(camera);
	portal->SetParentTransform(&board->GetTransform());
	portal->SetWarpTransform(warpPos);
	portals_.push_back(std::move(portal));
}