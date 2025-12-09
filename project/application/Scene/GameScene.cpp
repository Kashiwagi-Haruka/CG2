#include "GameScene.h"
#include "ModelManeger.h"
#include "ParticleManager.h"
#include "Object/Player/Player.h"
#include "CameraController.h"
#include "Object/Background/SkyDome.h"
#include "Object/Enemy/EnemyManager.h"
#include "SceneManager.h"
#include "ResultScene.h"
#include "GameOverScene.h"
GameScene::GameScene() {

	cameraController = new CameraController();
	particles = new Particles();
	skyDome = new SkyDome();
	player = new Player();
	bulletManager_ = new BulletManager();
	enemyManager = new EnemyManager();

	field = new MapchipField();
	goal = new Goal();
	sceneTransition = new SceneTransition();
	uimanager = new UIManager();
	BG = new Background();
	house = new House();


	BGMData = Audio::GetInstance()->SoundLoadFile("Resources/audio/昼下がり気分.mp3");
}
GameScene::~GameScene(){
	
}
void GameScene::Finalize() {

	Audio::GetInstance()->SoundUnload(&BGMData);
	for (int i = 0; i < 4; i++) {
		delete levelupIcons[i];
	}
	delete house;
	delete BG;
	delete uimanager;
	delete sceneTransition;
	delete goal;
	delete field;
	delete enemyManager;
	delete bulletManager_;
	delete player;
	delete skyDome;
	delete particles;
	delete cameraController;

}
void GameScene::Initialize() {

	sceneEndClear = false;
	sceneEndOver = false;

	cameraController->Initialize();
	
	GameBase::GetInstance()->SetDefaultCamera(cameraController->GetCamera());

	
	color = (uint8_t(meshColor.w * 255) << 24) | // A
	        (uint8_t(meshColor.x * 255) << 16) | // R
	        (uint8_t(meshColor.y * 255) << 8) |  // G
	        (uint8_t(meshColor.z * 255));        // B

	
	skyDome->Initialize(cameraController->GetCamera());
	player->Initialize(cameraController->GetCamera());
	bulletManager_->Initialize(cameraController->GetCamera());
	enemyManager->Initialize(cameraController->GetCamera());
	field->LoadFromCSV("Resources/CSV/MapChip_stage1.csv");
	field->Initialize(cameraController->GetCamera());
	goal->Initialize(cameraController->GetCamera());
	sceneTransition->Initialize();
	uimanager->SetPlayerHPMax(player->GetHPMax());
	uimanager->SetPlayerHP(player->GetHP());
	uimanager->SetPlayerPosition({player->GetPosition().x, player->GetPosition().y});
	uimanager->Initialize();
	BG->SetCamera(cameraController->GetCamera());
	BG->SetPosition(player->GetPosition());
	BG->Initialize();
	house->Initialize(cameraController->GetCamera());
	// ==================
	// ★ レベルアップ用スプライト画像読み込み
	// ==================
	
	int handle[4]{};
	handle[0] = TextureManager::GetInstance()->GetTextureIndexByfilePath("Resources/2d/levelup_attack.png");
	handle[1] = TextureManager::GetInstance()->GetTextureIndexByfilePath("Resources/2d/levelup_speed.png");
	handle[2] = TextureManager::GetInstance()->GetTextureIndexByfilePath("Resources/2d/levelup_hp.png");
	handle[3] = TextureManager::GetInstance()->GetTextureIndexByfilePath("Resources/2d/levelup_allow.png");

	for (int i = 0; i < 4; i++) {
	levelupIcons[i] = new Sprite();
		levelupIcons[i]->Initialize(GameBase::GetInstance()->GetSpriteCommon(), handle[i]);

		levelupIcons[i]->SetScale({256, 256});
	}

	Audio::GetInstance()->SoundPlayWave(BGMData);
}

void GameScene::Update() {
	// ★ レベルアップ選択中は操作受付
	if (isLevelSelecting) {

		// A で左、D で右
		if (GameBase::GetInstance()->TriggerKey(DIK_A)) {
			cursorIndex = 0;
		}
		if (GameBase::GetInstance()->TriggerKey(DIK_D)) {
			cursorIndex = 1;
		}

		// SPACEで決定
		if (GameBase::GetInstance()->TriggerKey(DIK_SPACE)) {

			int choice = selectChoices[cursorIndex];
			auto params = player->GetParameters();

			switch (choice) {
			case 0:
				params.AttuckUp++;
				break;
			case 1:
				params.SpeedUp++;
				break;
			case 2:
				params.HPUp++;
				break;
			case 3:
				params.AllowUp++;
				break;
			}

			player->SetParameters(params);

			player->IsLevelUp(false);
			isLevelSelecting = false;
		}

		// 選択中はその他の更新停止
		return;
	}

	
	skyDome->SetCamera(cameraController->GetCamera());
	player->SetCamera(cameraController->GetCamera());
	field->SetCamera(cameraController->GetCamera());
	goal->SetCamera(cameraController->GetCamera());
	BG->SetCamera(cameraController->GetCamera());
	bulletManager_->SetCamera(cameraController->GetCamera());
	
#ifdef USE_IMGUI
	//ImGui::Begin("Plane");
	//ImGui::Text("Transform");
	//ImGui::DragFloat3("Scale", &planeTransform.scale.x, 0.01f);
	//ImGui::DragFloat3("Rotate", &planeTransform.rotate.x, 0.01f);
	//ImGui::DragFloat3("Translate", &planeTransform.translate.x, 0.01f);
	//ImGui::ColorEdit4("Color", (float*)&meshColor);

	//if (ImGui::CollapsingHeader("light Settings")) {
	//	// 方向
	//	ImGui::DragFloat3("light Direction", &light.direction.x, 0.1f, -1.0f, 1.0f);
	//	// 明るさ
	//	ImGui::SliderFloat("light Intensity", &light.intensity, 0.0f, 5.0f);
	//	// 色
	//	ImGui::ColorEdit3("light Color", &light.color.x);
	//}

	//gameBase->SetDirectionalLightData(light);

	//// --- ブレンドモード選択 ---
	//static int blendModeIndex = 0;
	//const char* blendModes[] = {"None", "Alpha", "Add", "Sub", "Mul", "Screen"};
	//if (ImGui::Combo("Blend Mode", &blendModeIndex, blendModes, IM_ARRAYSIZE(blendModes))) {
	//	gameBase->SetBlendMode(static_cast<BlendMode>(blendModeIndex));
	//}

	//ImGui::End();

	

	/*ImGui::Begin("PadorKey");
	if (ImGui::Button("Keyboard")) {
		IsKeyboard = true;
		IsPKey = false;
	}
	if (ImGui::Button("Pad")) {
		IsXButton = false;
		IsKeyboard = false;
	}

	if (IsKeyboard) {
		ImGui::Text("Input = Keyboard");
	} else {
		ImGui::Text("Input = Pad");
	}

	ImGui::End();*/
	

	
	#endif
	

	ParticleManager::GetInstance()->Update(cameraController->GetCamera());
	skyDome->Update();
	bulletManager_->Update();
	player->SetBulletManager(bulletManager_);
	player->Update();
	// ===========================
	// ★ レベルアップを検知して選択画面へ
	// ===========================
	if (player->GetLv() && !isLevelSelecting) {

		isLevelSelecting = true;

		// ランダムで 0〜3 から2個選ぶ
		int a = rand() % 4;
		int b = rand() % 4;
		while (b == a) {
			b = rand() % 4;
		}

		selectChoices[0] = a;
		selectChoices[1] = b;

		cursorIndex = 0; // 左から開始

		// プレイヤーの動きを停止させたい場合はここで何もしない（GameScene が制御）
	}

	enemyManager->Update(cameraController->GetCamera());
	// ★★★ 敵が全滅したらゴールを表示・判定有効化 ★★★
	bool allDead = true;

	for (auto e : enemyManager->enemies) {
		if (e->GetIsAlive()) {
			allDead = false;
			break;
		}
	}

	goalActive = allDead;

	goal->Update(); 
	
	
	// 当たり判定サイズ（調整OK）
	float hitSize = 1.0f;
	// ===== プレイヤーとゴールの当たり判定 =====
	{
		Vector3 p = player->GetPosition();
		Vector3 g = goal->GetTranslate();

		float goalHitSize = 2.0f;

if (goalActive) { // ★ 敵全滅してからしか処理しない
			bool isGoalHit = fabs(p.x - g.x) < goalHitSize && fabs(p.y - g.y) < goalHitSize;

			if (isGoalHit) {
				BaseScene* scene = new ResultScene();
				SceneManager::GetInstance()->SetNextScene(scene);
			}
		}

	}

	
	if (!player->GetIsAlive()) {

		sceneEndOver = true;
	}

// ===== プレイヤーと敵の当たり判定 =====
	Vector3 p = player->GetPosition();
	Vector3 v = player->GetVelocity();
	
	Vector3 housePos = house->GetPosition();
	float houseHitSize = 8.0f;


	for (auto e : enemyManager->enemies) {

		if (!e->GetIsAlive())
			continue; // 死んだ敵はスキップ

		Vector3 ePos = e->GetPosition();

		// ===== ① プレイヤーが敵と接触 =====
		bool isCollidePlayer = fabs(p.x - ePos.x) < hitSize && fabs(p.y - ePos.y) < hitSize;

		if (isCollidePlayer) {

			// --- 上から踏んだ判定 ---
			bool isStomp = (p.y > ePos.y + 0.3f) && // プレイヤーが上にいる
			               (v.y < 0);               // 落下中

			if (isStomp) {
				e->Stun(); // 敵にダメージ（スタン）
			} else {
				player->Damage(1); // プレイヤーが被弾
			}
		}

		// ===== ② プレイヤー弾と敵の当たり判定 =====
		// ===== ② プレイヤー弾と敵の当たり判定 =====
		

			

			if (bulletManager_->Collision(ePos)) {
				e->SetHPSubtract(1*(player->GetParameters().AttuckUp+1)); 
				if (e->GetHP() <= 0) {
				    player->EXPMath();
				}
				
			}
		
			  // ===== House と敵の当たり判定 =====
		    bool hitHouse = fabs(ePos.x - housePos.x) < houseHitSize && fabs(ePos.y - housePos.y) < houseHitSize;

		    if (hitHouse) {
			    e->SetHPSubtract(10);      // ★ 敵を即死させる or 消す処理
			    house->Damage(1); // ★ house にダメージ

			    if (house->GetHP() <= 0) {
				    BaseScene* scene = new GameOverScene();
				    SceneManager::GetInstance()->SetNextScene(scene);
			    }
			    continue;
		    }
	}


	particles->SetPlayerPos(player->GetPosition());
	particles->SetCameraPos(cameraController->GetTransform().translate);
	particles->SetGoalPos(goal->GetTranslate());
	particles->Update();

	field->Update();
	uimanager->SetPlayerParameters(player->GetParameters());
	uimanager->SetPlayerHP(player->GetHP());
	uimanager->SetPlayerPosition({player->GetPosition().x, player->GetPosition().y});
	uimanager->SetHouseHP(house->GetHP());
	uimanager->SetHouseHPMax(30); // House の最大HP（好きに変更）

	uimanager->Update();
	BG->Update(player->GetPosition());
	house->Update(cameraController->GetCamera());

	// どちらかの HP が 0 以下で GameOver
	if (!player->GetIsAlive() || house->GetHP() <= 0) {
		sceneEndOver = true;
	}

	cameraController->SetTranslate({player->GetPosition().x, player->GetPosition().y + 5, cameraController->GetTransform().translate.z});
	cameraController->Update();
}

void GameScene::Draw() {

	GameBase::GetInstance()->ModelCommonSet();
	skyDome->Draw();
	player->Draw();
	bulletManager_->Draw();
	enemyManager->Draw();
	house->Draw();

	field->Draw();
	if (goalActive) {
		goal->Draw(); // ★ 敵全滅後だけ描画する
	}

	BG->Draw();
	ParticleManager::GetInstance()->Draw();
	
	GameBase::GetInstance()->SpriteCommonSet();
	uimanager->Draw();
	// =============================
	// ★ レベルアップ選択画面描画（別スプライト使用版）
	// =============================
	if (isLevelSelecting) {

		GameBase::GetInstance()->SpriteCommonSet();

		// 左と右の選択肢
		int leftID = selectChoices[0];
		int rightID = selectChoices[1];

		// 表示位置（自由に調整可）
		Vector2 leftPos = {350, 300};
		Vector2 rightPos = {750, 300};

		// 左アイコン描画
		levelupIcons[leftID]->SetPosition(leftPos);
		levelupIcons[leftID]->SetColor(cursorIndex == 0 ? Vector4(1, 1, 0, 1) : Vector4(1, 1, 1, 1));
		levelupIcons[leftID]->Update();
		levelupIcons[leftID]->Draw();

		// 右アイコン描画
		levelupIcons[rightID]->SetPosition(rightPos);
		levelupIcons[rightID]->SetColor(cursorIndex == 1 ? Vector4(1, 1, 0, 1) : Vector4(1, 1, 1, 1));
		levelupIcons[rightID]->Update();
		levelupIcons[rightID]->Draw();
	}


		/*if (IsKeyboard) {
	    if (gameBase->TriggerKey(DIK_P)) {
	        IsPKey = !IsPKey;
	    }
	    if (!IsPKey) {
	        if (!gameBase->PushKey(DIK_SPACE)) {
	            if (gameBase->PushKey(DIK_W)||gameBase->PushKey(DIK_A)||gameBase->PushKey(DIK_D)||gameBase->PushKey(DIK_S)) {

	                if (gameBase->PushKey(DIK_W)) {
	                    planeTransform.translate.y += 0.1f;
	                }
	                if (gameBase->PushKey(DIK_S)) {
	                    planeTransform.translate.y -= 0.1f;
	                }
	                if (gameBase->PushKey(DIK_A)) {
	                    planeTransform.translate.x -= 0.1f;
	                }
	                if (gameBase->PushKey(DIK_D)) {
	                    planeTransform.translate.x += 0.1f;
	                }

	            }
	            gameBase->DrawMesh(modelData.vertices, color, textureHandle, wvpMatrix, worldMatrix);
	        }
	    }
	} else {
	    if (gameBase->TriggerButton(Input::PadButton::kButtonX)) {
	        IsXButton = !IsXButton;
	    }
	    if (!IsXButton) {

	        if (!gameBase->PushButton(Input::PadButton::kButtonA)) {

	            if (gameBase->PushButton(Input::PadButton::kButtonUp)|| gameBase->PushButton(Input::PadButton::kButtonDown) || gameBase->PushButton(Input::PadButton::kButtonLeft) ||
	gameBase->PushButton(Input::PadButton::kButtonRight)) { if (gameBase->PushButton(Input::PadButton::kButtonUp)) { planeTransform.translate.y += 0.1f;
	                }
	                if (gameBase->PushButton(Input::PadButton::kButtonDown)) {
	                    planeTransform.translate.y -= 0.1f;
	                }
	                if (gameBase->PushButton(Input::PadButton::kButtonLeft)) {
	                    planeTransform.translate.x -= 0.1f;
	                }
	                if (gameBase->PushButton(Input::PadButton::kButtonRight)) {
	                    planeTransform.translate.x += 0.1f;
	                }
	            }

	            planeTransform.translate.x += gameBase->GetJoyStickLX() * 0.1f;
	            planeTransform.translate.y += gameBase->GetJoyStickLY() * 0.1f;

	            planeTransform.rotate.y += gameBase->GetJoyStickRX() * 0.1f;
	            planeTransform.rotate.x += gameBase->GetJoyStickRY() * 0.1f;

	            gameBase->DrawMesh(modelData.vertices, color, textureHandle, wvpMatrix, worldMatrix);
	        }
	    }
	}*/

	

}
