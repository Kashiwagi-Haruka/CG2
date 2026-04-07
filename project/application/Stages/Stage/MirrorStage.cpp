#include "MirrorStage.h"

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
	timeCardWatch_->SetPlayer(player_.get());

	// 懐中電灯
	flashlight_ = std::make_unique<Flashlight>();
	flashlight_->SetPlayer(player_.get());
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

void MirrorStage::Initialize() { isStageEnd_ = false;
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
	// 箱管理
	boxManager_->Initialize();
	// 最初のイベントをセットする
	currentEvent_ = firstEvent_.get();
	currentEvent_->StartEvent();
}

void MirrorStage::Update() { currentEvent_->Update(); }

void MirrorStage::Draw() {}

void MirrorStage::Finalize() {}

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