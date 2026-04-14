#include "MirrorStage.h"
#include "GameObject/Box/BoxManager.h"
#include "GameObject/Chair/ChairManager.h"
#include "GameObject/Coffee/Coffees.h"
#include "GameObject/Desk/DeskManager.h"
#include "GameObject/Door/Door.h"
#include "GameObject/Edamame/Edamame.h"
#include "GameObject/Flashlight/Flashlight.h"
#include "GameObject/Key/Key.h"
#include "GameObject/Locker/LockerManager.h"
#include "GameObject/PC/PC.h"
#include "GameObject/Player/Player.h"
#include "GameObject/Portal/PortalManager.h"
#include "GameObject/TestField/TestField.h"
#include "GameObject/TimeCard/TimeCard.h"
#include "GameObject/TimeCard/TimeCardRack.h"
#include "GameObject/TimeCard/TimeCardWatch.h"
#include "GameObject/VendingMac/VendingMac.h"
#include "GameObject/Wall/WallManager.h"
#include "GameObject/Wall/WallManager2.h"
#include "GameObject/WhiteBoard/WhiteBoardManager.h"
#include "GameObject/YoshidaMath/CollisionManager/CollisionManager.h"
#include <utility>

MirrorStage::MirrorStage(Player* player) : player_(player) {
	testField_ = std::make_unique<TestField>();
	whiteBoardManager_ = std::make_unique<WhiteBoardManager>(&player_->GetTransform().translate);
	portalManager_ = std::make_unique<PortalManager>(&player_->GetTransform().translate, whiteBoardManager_.get());
	collisionManager_ = std::make_unique<CollisionManager>();

	pc_ = std::make_unique<PC>();
	coffees_ = std::make_unique<Coffees>();
	timeCardWatch_ = std::make_unique<TimeCardWatch>();
	timeCardWatch_->SetPlayer(player_);
	flashlight_ = std::make_unique<Flashlight>();
	flashlight_->SetPlayer(player_);
	key_ = std::make_unique<Key>();
	edamame_ = std::make_unique<Edamame>();
	door_ = std::make_unique<Door>();
	lockerManager_ = std::make_unique<LockerManager>();
	wallManager_ = std::make_unique<WallManager>();
	wallManager2_ = std::make_unique<WallManager2>();
	vendingMac_ = std::make_unique<VendingMac>();
	chairManager_ = std::make_unique<ChairManager>();
	deskManager_ = std::make_unique<DeskManager>();
	timeCard_ = std::make_unique<TimeCard>();
	timeCardRack_ = std::make_unique<TimeCardRack>();
	boxManager_ = std::make_unique<BoxManager>();
}

void MirrorStage::Initialize() {

	testField_->Initialize();
	whiteBoardManager_->Initialize();
	portalManager_->Initialize();
	pc_->Initialize();
	coffees_->Initialize();
	coffees_->SetFloorY(0.0f);
	coffees_->SetRoomBounds(-40.0f, 40.0f, -40.0f, 40.0f);
	coffees_->SetSpawnContainment({0.0f, 0.0f, 0.0f}, 0.0f, 0.0f);
	timeCardWatch_->Initialize();
	flashlight_->Initialize();
	key_->Initialize();
	edamame_->Initialize();
	chairManager_->Initialize();
	wallManager_->Initialize();
	wallManager2_->Initialize();
	vendingMac_->Initialize();
	door_->Initialize();
	lockerManager_->Initialize();
	deskManager_->Initialize();
	timeCard_->Initialize();
	timeCardRack_->Initialize();
	boxManager_->Initialize();
	InitializeLights();
}
void MirrorStage::SetPlayer(Player* player) {
	player_ = player;
	timeCardWatch_->SetPlayer(player_);
	flashlight_->SetPlayer(player_);
}
void MirrorStage::UpdateGameObject(Camera* camera, const Vector3& lightDirection, Player* player) {
	portalManager_->WarpPlayer(player);
	timeCardWatch_->Update();
	flashlight_->Update();
	key_->Update();
	edamame_->Update();
	chairManager_->Update();
	boxManager_->Update();
	testField_->Update();
	wallManager_->Update();
	wallManager2_->Update();
	vendingMac_->Update();
	const Vector3 vendingPosition = vendingMac_->GetWorldPosition();
	const Vector3 vendingForward = vendingMac_->GetForward();
	coffees_->SetSpawnOrigin({
	    vendingPosition.x + vendingForward.x * 0.45f,
	    vendingPosition.y + 0.9f,
	    vendingPosition.z + vendingForward.z * 0.45f,
	});
	coffees_->SetLaunchDirection(vendingForward);
	coffees_->Update(camera, lightDirection);
	door_->Update();
	lockerManager_->Update();
	deskManager_->Update();
	whiteBoardManager_->Update();
	timeCard_->SetTransform({
	    {1.0f, 1.0f, 1.0f },
        {0.0f, 0.0f, 0.0f },
        {8.0f, 1.0f, -7.0f}
    });
	timeCard_->Update();
	timeCardRack_->SetTransform({
	    {1.0f,  1.0f, 1.0f },
        {0.0f,  0.0f, 0.0f },
        {7.75f, 1.3f, -7.0f}
    });
	timeCardRack_->Update();
	pc_->Update();
	UpdateLights();
}

void MirrorStage::InitializeLights() {
	assert(lightManager_);
	lightManager_->ClearLights();
	lightManager_->Initialize();
    lightManager_->SetActiveLightCount(Yoshida::LightManager::POINT, 2);

	lightManager_->SetPointLight(edamame_->GetPointLights().at(0),0);
	lightManager_->SetPointLight(edamame_->GetPointLights().at(1),1);

	//activePointLightCount_ = 4;
	//pointLights_[0].color = {1.0f, 1.0f, 1.0f, 1.0f};
	//pointLights_[0].position = {7.0f, 5.0f, 0.0f};
	//pointLights_[0].intensity = 1.0f;
	//pointLights_[0].radius = 10.0f;
	//pointLights_[0].decay = 1.0f;
	//pointLights_[1].color = {1.0f, 1.0f, 1.0f, 1.0f};
	//pointLights_[1].position = {-7.0f, 5.0f, 0.0f};
	//pointLights_[1].intensity = 1.0f;
	//pointLights_[1].radius = 10.0f;
	//pointLights_[1].decay = 1.0f;

	lightManager_->SetActiveLightCount(Yoshida::LightManager::SPOT, 1);
	lightManager_->SetSpotLight(flashlight_->GetSpotLight(),0);

	lightManager_->SetActiveLightCount(Yoshida::LightManager::AREA, 5);
	lightManager_->SetAreaLight(vendingMac_->GetAreaLight(),0);

	lightManager_->SetAreaLight(wallManager_->GetAreaLights().at(0),1);
	lightManager_->SetAreaLight(wallManager_->GetAreaLights().at(1),2);
	lightManager_->SetAreaLight(wallManager2_->GetAreaLights().at(0),3);
	lightManager_->SetAreaLight(wallManager2_->GetAreaLights().at(1),4);

}

void MirrorStage::UpdateLights() {

	//lightManager_->Update();

	lightManager_->SetSpotLight(flashlight_->GetSpotLight(), 0);

	//spotLights_[0] = flashlight_->GetSpotLight();
	//pointLights_[2] = edamame_->GetPointLights().at(0);
	//pointLights_[3] = edamame_->GetPointLights().at(1);
	//areaLights_[2] = vendingMac_->GetAreaLight();
	//areaLights_[3] = wallManager_->GetAreaLight();
	//areaLights_[4] = wallManager2_->GetAreaLight();

}
void MirrorStage::UpdatePortal() { portalManager_->Update(); }
void MirrorStage::SetCollisionManager(CollisionManager* collisionManager) {
	stageCollisionManager_ = collisionManager;
}
void MirrorStage::CheckCollision() {
	if (!stageCollisionManager_) {
		return;
	}

	portalManager_->CheckCollision();
	door_->CheckCollision();

	if (vendingMac_->ConsumeInteractRequest()) {
		coffees_->StartSpill();
	}

	for (auto& portal : portalManager_->GetPortals()) {
		if (!portal->GetIsPlayerHit()) {
			stageCollisionManager_->AddCollider(portal.get());
		} else {
			break;
		}
	}

	for (auto& whiteBoard : whiteBoardManager_->GetWhiteBoards()) {
		stageCollisionManager_->AddCollider(whiteBoard.get());
	}
	for (auto& wall : wallManager_->GetWalls()) {
		stageCollisionManager_->AddCollider(wall.get());
	}
	for (auto& wall : wallManager2_->GetWalls()) {
		stageCollisionManager_->AddCollider(wall.get());
	}
	for (auto& chair : chairManager_->GetChairs()) {
		stageCollisionManager_->AddCollider(chair.get());
	}
	for (auto& locker : lockerManager_->GetLockers()) {
		stageCollisionManager_->AddCollider(locker.get());
	}
	for (auto& desk : deskManager_->GetDesks()) {
		stageCollisionManager_->AddCollider(desk.get());
	}
	for (auto& box : boxManager_->GetBoxes()) {
		stageCollisionManager_->AddCollider(box.get());
	}

	stageCollisionManager_->AddCollider(vendingMac_.get());
	stageCollisionManager_->AddCollider(flashlight_.get());
	stageCollisionManager_->AddCollider(testField_.get());
	stageCollisionManager_->AddCollider(door_->GetAutoLockSystem().get());
	if (!door_->GetIsOpen()) {
		stageCollisionManager_->AddCollider(door_.get());
	}
	stageCollisionManager_->AddCollider(key_.get());
	stageCollisionManager_->AddCollider(edamame_->GetEdamameModel().get());
	stageCollisionManager_->AddCollider(pc_.get());
	stageCollisionManager_->CheckAllCollisions();
}

void MirrorStage::DrawModel(bool isShadow, bool drawPortal, bool isDrawParticle) {
	testField_->Draw();
	wallManager_->Draw();
	wallManager2_->Draw();
	vendingMac_->Draw();
	coffees_->Draw();
	timeCardWatch_->Draw();
	flashlight_->Draw();
	key_->Draw();
	chairManager_->Draw();
	timeCard_->Draw();
	timeCardRack_->Draw();
	boxManager_->Draw();
	lockerManager_->Draw();
	pc_->Draw();
	door_->Draw();
	deskManager_->Draw();
	edamame_->Draw();
	whiteBoardManager_->Draw();
	portalManager_->Draw(isShadow, drawPortal, isDrawParticle);
}

void MirrorStage::SetSceneCameraForDraw(Camera* camera) {
	testField_->SetCamera(camera);
	whiteBoardManager_->SetCamera(camera);
	portalManager_->SetCamera(camera);
	pc_->SetCamera(camera);
	timeCardWatch_->SetCamera(camera);
	flashlight_->SetCamera(camera);
	key_->SetCamera(camera);
	edamame_->SetCamera(camera);
	chairManager_->SetCamera(camera);
	wallManager_->SetCamera(camera);
	wallManager2_->SetCamera(camera);
	vendingMac_->SetCamera(camera);
	door_->SetCamera(camera);
	lockerManager_->SetCamera(camera);
	deskManager_->SetCamera(camera);
	timeCard_->SetCamera(camera);
	timeCardRack_->SetCamera(camera);
	boxManager_->SetCamera(camera);
}

void MirrorStage::SetPlayerCamera(PlayerCamera* playerCamera) {
	portalManager_->SetPlayerCamera(playerCamera);
	key_->SetPlayerCamera(playerCamera);
	edamame_->SetPlayerCamera(playerCamera);
	chairManager_->SetPlayerCamera(playerCamera);
	vendingMac_->SetPlayerCamera(playerCamera);
	door_->SetPlayerCamera(playerCamera);
	flashlight_->SetPlayerCamera(playerCamera);
	lockerManager_->SetPlayerCamera(playerCamera);
	deskManager_->SetPlayerCamera(playerCamera);
	boxManager_->SetPlayerCamera(playerCamera);
	pc_->SetPlayerCamera(playerCamera);
}

PortalManager* MirrorStage::GetPortalManager() { return portalManager_.get(); }

std::unique_ptr<CollisionManager> MirrorStage::GetCollisionManager() { return std::move(collisionManager_); }

void MirrorStage::SetLightManager(Yoshida::LightManager* lightManager)
{  lightManager_ = lightManager; 
}
