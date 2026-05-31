#include "GiantEnemyManager.h"
#include "Model/ModelManager.h"

bool GiantEnemyManager::isAllPortal_ = false;
GiantEnemyManager::GiantEnemyManager(Vector3* playerPos, const uint32_t createNum)
{

    ModelManager::GetInstance()->LoadGltfModel("Resources/TD3_3102/3d/gentleman", "gentleman");

    for (uint32_t i = 0; i < createNum; ++i) {
        std::unique_ptr<GiantEnemy> enemy = std::make_unique<GiantEnemy>();
        enemy->SetName("GiantEnemy" + std::to_string(i));
        giantEnemies_.push_back(std::move(enemy));
    }

    GiantEnemy::SetTargetPos(playerPos);
    createNum_ = createNum;
}

GiantEnemyManager::~GiantEnemyManager()
{
    for (auto& enemy : giantEnemies_) {
        enemy.reset();
    }
    giantEnemies_.clear();
}

void GiantEnemyManager::Initialize()
{
    isAllPortal_ = false;

    for (auto& enemy : giantEnemies_) {
        enemy->Initialize();
    }

    pregiantEnemies_.clear();

}

void GiantEnemyManager::Update()
{
    for (auto& enemy : giantEnemies_) {
        enemy->Update();
    }
}

void GiantEnemyManager::Draw()
{
    for (auto& enemy : giantEnemies_) {
        enemy->Draw();
    }
}

void GiantEnemyManager::SetCamera(Camera* camera)
{
    for (auto& enemy : giantEnemies_) {
        enemy->SetCamera(camera);
    }
}

GiantEnemy* GiantEnemyManager::CheckCollision(PlayerCamera* playerCamera)
{
    if (!playerCamera) {
        return nullptr;
    }

    for (auto& enemy : giantEnemies_) {
   
        if (!enemy->OnCollisionRay(playerCamera)) {
            continue;
        }

        //ポータルとカメラが向き合っているかどうか
        if (!enemy->IsFacingSurface(playerCamera->GetCamera()->GetWorldMatrix())) {
            continue;
        }

        if (enemy->IsWall()) {
            //既に壁なら
            continue;
        }
  /*      if (enemy->GetCollisionAttribute() == kCollisionWall) {
         
        }*/

        //ボード返すよーん
        return enemy.get();
    }

    return nullptr;
}


void GiantEnemyManager::SetPortal(GiantEnemy* enemy)
{

    //if (preWhiteBoards_.size() >= 2) {
    //    // ポータルの生成が2個以上になったら
    //    preWhiteBoards_.at(0)->ResetCollisionAttribute();
    //    preWhiteBoards_.erase(preWhiteBoards_.begin());
    //}

    pregiantEnemies_.push_back(enemy);

    // マスクの設定とフラグの初期化
    pregiantEnemies_.back()->SetCollisionAttributeWallAndInitialize();

    isAllPortal_ = IsAllPortal();
}

void GiantEnemyManager::CheckFloorCollision(YoshidaMath::Collider* collider)
{
    for (auto& enemy : giantEnemies_) {
        enemy->OnCollisionWithFloor(collider);
    }
}
