#pragma once

#include"GiantEnemy.h"
#include "Vector3.h"
#include <memory>
#include <vector>
#include<stdint.h>

class Camera;
class PlayerCamera;

class GiantEnemyManager
{
public:
    GiantEnemyManager(Vector3* playerPos, const uint32_t createNum = 4);
	uint32_t createNum_ = 0;
	~GiantEnemyManager();
    void Initialize();
	void Update();
	void Draw();
	void SetCamera(Camera* camera);
	bool IsAllPortal() { return createNum_ <= static_cast<uint32_t>(pregiantEnemies_.size()); };
	static bool GetIsAllPortal() { return isAllPortal_; }
	/// @brief レイとホワイトボードの判定を行い、ポータル生成対象を返す
	/// @return ポータル生成対象のホワイトボード。生成不要時はnullptr
	GiantEnemy* CheckCollision(PlayerCamera* playerCamera);
	void SetPortal(GiantEnemy* enemy);
	void CheckFloorCollision(YoshidaMath::Collider* collider);
	std::vector<std::unique_ptr<GiantEnemy>>& GetEnemies() { return giantEnemies_; }
private:
	static bool isAllPortal_;
	std::vector<std::unique_ptr<GiantEnemy>>giantEnemies_;
	std::vector<GiantEnemy*> pregiantEnemies_;
};

