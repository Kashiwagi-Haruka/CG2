#pragma once

#include "GameObject/WhiteBoard/WhiteBoard.h"
#include "Vector3.h"
#include <memory>
#include <vector>

class Camera;
class PlayerCamera;

class WhiteBoardManager {
public:
	explicit WhiteBoardManager(Vector3* playerPos);
	~WhiteBoardManager();

	void Initialize();
	void Update();
	void Draw();
	void SetCamera(Camera* camera);

	/// @brief レイとホワイトボードの判定を行い、ポータル生成対象を返す
	/// @return ポータル生成対象のホワイトボード。生成不要時はnullptr
	WhiteBoard* CheckCollision(PlayerCamera* playerCamera);
	bool GetCanMakePortal() const { return canMakePortal_; }

	std::vector<std::unique_ptr<WhiteBoard>>& GetWhiteBoards() { return whiteBoards_; }

private:
	bool OnCollisionRay(PlayerCamera* playerCamera, const AABB& aabb, const Vector3& pos);

private:
	Vector3* playerPos_ = nullptr;
	bool canMakePortal_ = false;
	std::vector<WhiteBoard*> preWhiteBoards_;
	std::vector<std::unique_ptr<WhiteBoard>> whiteBoards_;
};