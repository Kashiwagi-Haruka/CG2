#pragma once

#include "Stages/BaseStage.h"
#include"GameObject/TestField/TestField.h"

#include"GameObject/Portal/PortalManager.h"
#include"GameObject/WhiteBoard/WhiteBoardManager.h"
#include"GameObject/TimeCard/TimeCardWatch.h"
#include"GameObject/TimeCard/TimeCard.h"
#include"GameObject/TimeCard/TimeCardRack.h"
#include"GameObject/Flashlight/Flashlight.h"
#include"GameObject/Key/Key.h"
#include <GameObject/Chair/ChairManager.h>
#include"GameObject/Wall/WallManager.h"
#include"GameObject/Wall/WallManager2.h"

#include"GameObject/VendingMac/VendingMac.h"
#include"GameObject/Door/Door.h"
#include"GameObject/Locker/LockerManager.h"
#include"GameObject/Desk/DeskManager.h"
#include"GameObject/Box/BoxManager.h"
#include "Menu/Menu.h"

#include <GameObject/Edamame/Edamame.h>
#include"GameObject/PC/PC.h"

#include "GameObject/Coffee/Coffees.h"



#pragma region // Event
#include "GameObject/Event/FirstGameEvent.h"
#pragma endregion
#include"GameObject/YoshidaMath/CollisionManager/CollisionManager.h"

class MirrorStage : public BaseStage {

	   // TestField
	std::unique_ptr<TestField> testField_ = nullptr;
	// ホワイトボード管理
	std::unique_ptr<WhiteBoardManager> whiteBoardManager_ = nullptr;
	// ポータル管理
	std::unique_ptr<PortalManager> portalManager_ = nullptr;
	// 携帯打刻機
	std::unique_ptr<TimeCardWatch> timeCardWatch_ = nullptr;
	// 懐中電灯
	std::unique_ptr<Flashlight> flashlight_ = nullptr;
	// 鍵
	std::unique_ptr<Key> key_ = nullptr;
	// 枝豆
	std::unique_ptr<Edamame> edamame_ = nullptr;
	// 椅子
	std::unique_ptr<ChairManager> chairManager_ = nullptr;
	// 壁管理
	std::unique_ptr<WallManager> wallManager_ = nullptr;
	// 壁管理2こめ
	std::unique_ptr<WallManager2> wallManager2_ = nullptr;

	    // 自販機
	std::unique_ptr<VendingMac> vendingMac_ = nullptr;
	// ドア
	std::unique_ptr<Door> door_ = nullptr;
	// ロッカー管理
	std::unique_ptr<LockerManager> lockerManager_ = nullptr;
	// デスク管理
	std::unique_ptr<DeskManager> deskManager_ = nullptr;
	// タイムカード
	std::unique_ptr<TimeCard> timeCard_ = nullptr;
	std::unique_ptr<TimeCardRack> timeCardRack_ = nullptr;
	// 箱
	std::unique_ptr<BoxManager> boxManager_ = nullptr;

	   // PC
	std::unique_ptr<PC> pc_ = nullptr;
	// コーヒー缶
	std::unique_ptr<Coffees> coffees_ = nullptr;

	// 衝突管理
	std::unique_ptr<CollisionManager> collisionManager_ = nullptr;
	// 最初のイベント
	std::unique_ptr<FirstGameEvent> firstEvent_ = nullptr;
	GameEvent* currentEvent_ = nullptr;

public:

	MirrorStage();
	void Initialize() override;
	void Update() override;
	void Draw() override;
	void Finalize() override;

	private:
	void CheckCollision();


};