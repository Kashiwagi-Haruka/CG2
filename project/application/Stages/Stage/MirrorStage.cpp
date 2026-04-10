#define NOMINMAX
#include "MirrorStage.h"
#include "Engine/base/DirectXCommon.h"
#include <imgui.h>
#include "GameObject/Player/Player.h"
#include "Object3d/Object3dCommon.h"
#include "GameObject/Elevator/ElevatorRoomManager.h"
#include "GameObject/SEManager/SEManager.h"
#include <algorithm>
#include <cmath>
MirrorStage::MirrorStage() {

 // テスト地面
	testField_ = std::make_unique<TestField>();
	// ホワイトボード管理
	whiteBoardManager_ = std::make_unique<WhiteBoardManager>(&player_->GetTransform().translate);
	// ポータル管理
	portalManager_ = std::make_unique<PortalManager>(&player_->GetTransform().translate, whiteBoardManager_.get());

	// PC
	pc_ = std::make_unique<PC>();
	// コーヒー缶
	coffees_ = std::make_unique<Coffees>();
	// 携帯打刻機
	timeCardWatch_ = std::make_unique<TimeCardWatch>();
	timeCardWatch_->SetPlayer(player_);

	// 懐中電灯
	flashlight_ = std::make_unique<Flashlight>();
	flashlight_->SetPlayer(player_);
	// 鍵管理
	key_ = std::make_unique<Key>();
	// 枝豆管理
	edamame_ = std::make_unique<Edamame>();
	// ドア
	door_ = std::make_unique<Door>();
	// ロッカー
	lockerManager_ = std::make_unique<LockerManager>();
	// 壁管理
	wallManager_ = std::make_unique<WallManager>();
	// 壁管理
	wallManager2_ = std::make_unique<WallManager2>();
	// 自販機
	vendingMac_ = std::make_unique<VendingMac>();
	// 椅子
	chairManager_ = std::make_unique<ChairManager>();
	// 机
	deskManager_ = std::make_unique<DeskManager>();
	// 打刻機
	timeCard_ = std::make_unique<TimeCard>();
	// タイムカードラック
	timeCardRack_ = std::make_unique<TimeCardRack>();
	// 箱管理
	boxManager_ = std::make_unique<BoxManager>();
	// 衝突管理
	collisionManager_ = std::make_unique<CollisionManager>();

	// 最初のイベント
	firstEvent_ = std::make_unique<FirstGameEvent>();
}

void MirrorStage::Initialize() {
	isStageEnd_ = false;
	playerHp_ = kPlayerMaxHp_;
	damageCooldownTimer_ = 0.0f;
	didTakeDamage_ = false;
	noiseTimer_ = kNoiseTimer_;
	isNoise_ = false;
	// テスト地面
	testField_->Initialize();
	// ホワイトボード管理
	whiteBoardManager_->Initialize();
	// ポータル管理
	portalManager_->Initialize();
	// PC
	pc_->Initialize();
	// コーヒー缶
	coffees_->Initialize();
	coffees_->SetFloorY(0.0f);
	coffees_->SetRoomBounds(-40.0f, 40.0f, -40.0f, 40.0f);
	coffees_->SetSpawnContainment({0.0f, 0.0f, 0.0f}, 0.0f, 0.0f);
	// 携帯打刻機
	timeCardWatch_->Initialize();
	// 鍵
	key_->Initialize();
	// 枝豆
	edamame_->Initialize();
	// 椅子
	chairManager_->Initialize();
	// 壁
	wallManager_->Initialize();
	// 壁
	wallManager2_->Initialize();
	// 自販機
	vendingMac_->Initialize();
	// ドア
	door_->Initialize();
	// ロッカー
	lockerManager_->Initialize();
	// デスク管理
	deskManager_->Initialize();
	// 打刻機
	timeCard_->Initialize();
	// タイムカードラック
	timeCardRack_->Initialize();
	flashlight_->Initialize();
	// 箱管理
	boxManager_->Initialize();
	// 最初のイベントをセットする
	currentEvent_ = firstEvent_.get();
	currentEvent_->StartEvent();
}

void MirrorStage::Update() {
	didTakeDamage_ = false;
	if (currentEvent_) {
		currentEvent_->Update();
	}

	portalManager_->WarpPlayer(player_);

	// 携帯打刻機
	timeCardWatch_->Update();
	// 懐中電灯
	flashlight_->Update();
	// 鍵
	key_->Update();
	// 枝豆
	edamame_->Update();
	// 椅子
	chairManager_->Update();
	// 箱
	boxManager_->Update();
	// 床
	testField_->Update();
	// 壁
	wallManager_->Update();
	wallManager2_->Update();
	// 自販機
	vendingMac_->Update();
	const Vector3 vendingPosition = vendingMac_->GetWorldPosition();
	const Vector3 vendingForward = vendingMac_->GetForward();
	coffees_->SetSpawnOrigin({
	    vendingPosition.x + vendingForward.x * 0.45f,
	    vendingPosition.y + 0.9f,
	    vendingPosition.z + vendingForward.z * 0.45f,
	});
	coffees_->SetLaunchDirection(vendingForward);
	coffees_->Update(Object3dCommon::GetInstance()->GetDefaultCamera(), {0.0f, 1.0f, 0.0f});
	UpdatePlayerDamage();
	// ドア
	door_->Update();
	// ロッカー
	lockerManager_->Update();
	// 机
	deskManager_->Update();
	// ホワイトボードとポータル
	whiteBoardManager_->Update();
	portalManager_->Update();
	// 打刻機
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
	// PC
	pc_->Update();

	CheckCollision();
}

void MirrorStage::UpdatePlayerDamage() {
	const float deltaTime = Object3dCommon::GetInstance()->GetDxCommon()->GetDeltaTime();
	damageCooldownTimer_ = std::max(0.0f, damageCooldownTimer_ - deltaTime);

	constexpr float kHpRegenPerSecond = 0.2f;
	playerHp_ = std::min(kPlayerMaxHp_, playerHp_ + (kHpRegenPerSecond * deltaTime));

	constexpr float kCoffeeHitSpeedThreshold = 2.0f;
	constexpr float kPlayerHitRadius = 0.45f;
	constexpr float kDamageCooldown = 0.7f;

	if (damageCooldownTimer_ <= 0.0f && coffees_->CheckHitPlayer(player_->GetWorldPosition(), kPlayerHitRadius, kCoffeeHitSpeedThreshold)) {
		ApplyPlayerDamage(1.0f);
		damageCooldownTimer_ = kDamageCooldown;
	}
}

void MirrorStage::ApplyPlayerDamage(float damageAmount) {
	const float prevHp = playerHp_;
	playerHp_ = std::max(0.0f, playerHp_ - damageAmount);
	if (playerHp_ < prevHp) {
		didTakeDamage_ = true;
		SEManager::SoundPlay(SEManager::DAMAGE);
	}
	if (playerHp_ > 1.0f && playerHp_ < kPlayerMaxHp_) {
		playerHp_ = std::floor(playerHp_);
	}
}

void MirrorStage::Draw() {
	UpdateLight();
	UpdatePostEffect();
	DrawModel();
}

void MirrorStage::Finalize() {}
void MirrorStage::SetPlayer(Player* player) {
	player_ = player;
	if (timeCardWatch_) {
		timeCardWatch_->SetPlayer(player_);
	}
	if (flashlight_) {
		flashlight_->SetPlayer(player_);
	}
}
bool MirrorStage::IsCurrentEventRunning() const { return currentEvent_ != nullptr && currentEvent_->IsRunning(); }
void MirrorStage::CheckCollision() {
	// ホワイトボードとrayの当たり判定作成する
	portalManager_->CheckCollision();
	door_->CheckCollision();
	// コーヒー排出する
	if (vendingMac_->ConsumeInteractRequest()) {
		coffees_->StartSpill();
	}

	collisionManager_->ClearColliders();

	for (auto& portal : portalManager_->GetPortals()) {
		if (!portal->GetIsPlayerHit()) {
			// プレイヤーがヒットしてないときコライダーリストに追加する
			collisionManager_->AddCollider(portal.get());
		} else {
			break;
		}
	}

	for (auto& whiteBoard : whiteBoardManager_->GetWhiteBoards()) {
		collisionManager_->AddCollider(whiteBoard.get());
	}
	for (auto& wall : wallManager_->GetWalls()) {
		collisionManager_->AddCollider(wall.get());
	}

	for (auto& wall : wallManager2_->GetWalls()) {
		collisionManager_->AddCollider(wall.get());
	}
	for (auto& chair : chairManager_->GetChairs()) {
		collisionManager_->AddCollider(chair.get());
	}
	for (auto& locker : lockerManager_->GetLockers()) {
		collisionManager_->AddCollider(locker.get());
	}
	for (auto& desk : deskManager_->GetDesks()) {
		collisionManager_->AddCollider(desk.get());
	}
	for (auto& box : boxManager_->GetBoxes()) {
		collisionManager_->AddCollider(box.get());
	}
	if (elevatorRoomManager_) {
		for (auto& wall : elevatorRoomManager_->GetWalls()) {
			collisionManager_->AddCollider(wall.get());
		}
	}

	collisionManager_->AddCollider(vendingMac_.get());
	collisionManager_->AddCollider(flashlight_.get());
	collisionManager_->AddCollider(testField_.get());
	collisionManager_->AddCollider(door_->GetAutoLockSystem().get());
	if (!door_->GetIsOpen()) {
		collisionManager_->AddCollider(door_.get());
	}

	collisionManager_->AddCollider(key_.get());
	collisionManager_->AddCollider(edamame_->GetEdamameModel().get());
	collisionManager_->AddCollider(pc_.get());
}
void MirrorStage::DrawGameObject(bool isShadow, bool drawPortal, bool isDrawParticle) {
	// テスト地面
	testField_->Draw();
	// 壁管理
	wallManager_->Draw();
	// 壁管理
	wallManager2_->Draw();
	// 自販機
	vendingMac_->Draw();
	// コーヒー缶
	coffees_->Draw();
	// ドア
	door_->Draw();
	// ロッカー
	lockerManager_->Draw();
	// 机
	deskManager_->Draw();
	// PC
	pc_->Draw();
	// 携帯打刻機の描画処理
	timeCardWatch_->Draw();
	// 懐中電灯
	flashlight_->Draw();
	// 鍵の描画処理
	key_->Draw();
	// 枝豆の描画処理
	edamame_->Draw();
	// 椅子の描画
	chairManager_->Draw();
	// 打刻機
	timeCard_->Draw();
	// タイムカードラック
	timeCardRack_->Draw();
	// 箱管理
	boxManager_->Draw();
	// エレベータールーム
	if (elevatorRoomManager_) {
		elevatorRoomManager_->Draw();
	}

	// ホワイトボード管理の描画
	whiteBoardManager_->Draw();
	// ポータル管理の描画
	portalManager_->Draw(isShadow, drawPortal, isDrawParticle);
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
	if (elevatorRoomManager_) {
		elevatorRoomManager_->SetCamera(camera);
	}
}
void MirrorStage::SetCameraAndDraw(Camera* camera, bool drawPortal, bool isDrawParticle) {
	Object3dCommon::GetInstance()->SetDefaultCamera(camera);
	SetSceneCameraForDraw(camera);
	Object3dCommon::GetInstance()->DrawCommon();
	DrawGameObject(false, drawPortal, isDrawParticle);
}
void MirrorStage::InitializeLights() {

	    // フラッシュライト
	flashlight_->Initialize();

	spotLights_[0] = flashlight_->GetSpotLight();

	areaLights_[2] = vendingMac_->GetAreaLight();

	activePointLightCount_ = 3;
	pointLights_[0].color = {1.0f, 1.0f, 1.0f, 1.0f};
	pointLights_[0].position = {7.0f, 5.0f, 0.0f};
	pointLights_[0].intensity = 1.0f;
	pointLights_[0].radius = 10.0f;
	pointLights_[0].decay = 1.0f;
	pointLights_[1].color = {1.0f, 1.0f, 1.0f, 1.0f};
	pointLights_[1].position = {5.0f, 5.0f, 5.0f};
	pointLights_[1].intensity = 1.0f;
	pointLights_[1].radius = 10.0f;
	pointLights_[1].decay = 1.0f;

	pointLights_[2] = edamame_->GetPointLight();
	activeSpotLightCount_ = 1;

	activeAreaLightCount_ = 6;
	areaLights_[0].color = {1.0f, 1.0f, 1.0f, 1.0f};
	areaLights_[0].position = {7.0f, 3.0f, 0.0f};
	areaLights_[0].normal = {0.0f, 1.0f, 0.0f};
	areaLights_[0].intensity = 10.0f;
	areaLights_[0].width = 4.0f;
	areaLights_[0].height = 0.1f;
	areaLights_[0].radius = 4.0f;
	areaLights_[0].decay = 2.0f;

	areaLights_[1].color = {1.0f, 1.0f, 1.0f, 1.0f};
	areaLights_[1].position = {-7.0f, 3.0f, 0.0f};
	areaLights_[1].normal = {0.0f, 1.0f, 0.0f};
	areaLights_[1].intensity = 10.0f;
	areaLights_[1].width = 4.0f;
	areaLights_[1].height = 0.1f;
	areaLights_[1].radius = 4.0f;
	areaLights_[1].decay = 2.0f;
}
void MirrorStage::SetElevatorManager(ElevatorRoomManager* elevatorRoomManager) {
	elevatorRoomManager_ = elevatorRoomManager;
}
void MirrorStage::UpdateLight() {
	spotLights_[0] = flashlight_->GetSpotLight();

	pointLights_[2] = edamame_->GetPointLight();

	areaLights_[2] = vendingMac_->GetAreaLight();
	areaLights_[3] = wallManager_->GetAreaLight();
	areaLights_[4] = wallManager2_->GetAreaLight();
	if (elevatorRoomManager_) {
		areaLights_[5] = elevatorRoomManager_->GetAreaLight();
	}
	Object3dCommon::GetInstance()->SetPointLights(pointLights_.data(), activePointLightCount_);
	Object3dCommon::GetInstance()->SetSpotLights(spotLights_.data(), activeSpotLightCount_);
	Object3dCommon::GetInstance()->SetAreaLights(areaLights_.data(), activeAreaLightCount_);
	Object3dCommon::GetInstance()->SetShadowMapEnabled(useDirectionalShadow_, usePointShadow_, useSpotShadow_, useAreaShadow_);
#ifdef USE_IMGUI
	if (ImGui::TreeNode("Light")) {
		ImGui::DragFloat3("Area0Position", &areaLights_[0].position.x, 0.1f);
		ImGui::DragFloat3("Area1Position", &areaLights_[1].position.x, 0.1f);
		ImGui::Checkbox("PointShadow", &usePointShadow_);
		ImGui::Checkbox("SpotShadow", &useSpotShadow_);
		ImGui::Checkbox("AreaShadow", &useAreaShadow_);
		pointLights_[0].shadowEnabled = usePointShadow_ ? 1 : 0;
		spotLights_[0].shadowEnabled = useSpotShadow_ ? 1 : 0;
		areaLights_[0].shadowEnabled = useAreaShadow_ ? 1 : 0;
		ImGui::TreePop();
	}
#endif
}
void MirrorStage::UpdatePostEffect() {
	bool vignetteStrength = true;

	Object3dCommon::GetInstance()->SetFullScreenGrayscaleEnabled(false);
	Object3dCommon::GetInstance()->SetFullScreenSepiaEnabled(false);
	Object3dCommon::GetInstance()->GetDxCommon()->SetVignetteStrength(vignetteStrength);
	Object3dCommon::GetInstance()->SetVignetteStrength(vignetteStrength);

	for (auto& portal : portalManager_->GetPortals()) {
		if (portal->GetIsPlayerHit()) {
			if (!isNoise_) {
				isNoise_ = true;
			}
		}
	}

	if (isNoise_) {
		float randomNoiseScale = 1.0f;
		noiseTimer_ -= YoshidaMath::kDeltaTime;
		if (noiseTimer_ <= 0.0f) {
			isNoise_ = false;
			noiseTimer_ = kNoiseTimer_;
		}
	}
	BlendMode randomNoiseBlendMode = kBlendModeSub;

	Object3dCommon::GetInstance()->SetRandomNoiseEnabled(isNoise_);
	Object3dCommon::GetInstance()->SetRandomNoiseScale(noiseTimer_);
	Object3dCommon::GetInstance()->SetRandomNoiseBlendMode(randomNoiseBlendMode);
}
void MirrorStage::DrawModel() {
	//=======================shadowマップの開始↓=======================
	auto* object3dCommon = Object3dCommon::GetInstance();
	Camera* mainCamera = object3dCommon->GetDefaultCamera();
	const bool shadowFlags[4] = {useDirectionalShadow_, usePointShadow_, useSpotShadow_, useAreaShadow_};
	for (int i = 0; i < 4; ++i) {
		if (!shadowFlags[i]) {
			continue;
		}
		object3dCommon->SetShadowMapEnabled(i == 0, i == 1, i == 2, i == 3);
		object3dCommon->BeginShadowMapPass();
		object3dCommon->DrawCommonShadow();
		DrawGameObject(true, false, false);
		object3dCommon->EndShadowMapPass();
	}
	object3dCommon->SetShadowMapEnabled(useDirectionalShadow_, usePointShadow_, useSpotShadow_, useAreaShadow_);
	//=======================shadowマップの終了↑=======================

	for (auto& portal : portalManager_->GetPortals()) {
		portal->BeginRender();
		auto* portalCamera = portal->GetCamera();
		SetCameraAndDraw(portalCamera, false, false);
		portal->TransitionToShaderResource();
	}
	if (mainCamera) {
		object3dCommon->GetDxCommon()->SetMainRenderTarget();
		SetCameraAndDraw(mainCamera, true, true);
	}
}