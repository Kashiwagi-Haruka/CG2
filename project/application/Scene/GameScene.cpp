#include "GameScene.h"
#include "ModelManeger.h"
#include "ParticleManager.h"
#include "Object/Player/Player.h"
#include "CameraController.h"
#include "Object/Background/SkyDome.h"
#include "Object/Enemy/EnemyManager.h"
GameScene::GameScene() {

	cameraController = new CameraController();
	particles = new Particles();
	skyDome = new SkyDome();
	player = new Player();
	enemyManager = new EnemyManager();

	field = new MapchipField();
	goal = new Goal();
	sceneTransition = new SceneTransition();
	uimanager = new UIManager();
	BG = new Background();
	soundData = Audio::GetInstance()->SoundLoadFile("Resources/audio/Alarm01.wav");
	BGMData = Audio::GetInstance()->SoundLoadFile("Resources/audio/昼下がり気分.mp3");
}
GameScene::~GameScene(){
	Audio::GetInstance()->SoundUnload(&BGMData);
	Audio::GetInstance()->SoundUnload(&soundData);
	delete BG;
	delete uimanager;
	delete sceneTransition;
	delete goal;  
	delete field;
	delete enemyManager;
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
	
	Audio::GetInstance()->SoundPlayWave(soundData);
	Audio::GetInstance()->SoundPlayWave(BGMData);
}

void GameScene::Update() {

	
	skyDome->SetCamera(cameraController->GetCamera());
	player->SetCamera(cameraController->GetCamera());
	field->SetCamera(cameraController->GetCamera());
	goal->SetCamera(cameraController->GetCamera());
	BG->SetCamera(cameraController->GetCamera());
	
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
	player->Update();
	enemyManager->Update(cameraController->GetCamera());

	goal->Update(); 

	
	// 当たり判定サイズ（調整OK）
	float hitSize = 1.0f;
	// ===== プレイヤーとゴールの当たり判定 =====
	{
		Vector3 p = player->GetPosition();
		Vector3 g = goal->GetTranslate();

		float goalHitSize = 2.0f;

		bool isGoalHit = fabs(p.x - g.x) < goalHitSize && fabs(p.y - g.y) < goalHitSize;

		if (isGoalHit) {
			sceneEndClear = true;
		}
	}

	
	if (!player->GetIsAlive()) {

		sceneEndOver = true;
	}

// ===== プレイヤーと敵の当たり判定 =====
	Vector3 p = player->GetPosition();
	Vector3 v = player->GetVelocity();
	
	float bulletHitSize = 1.0f; // 弾の当たり判定

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
		if (player->GetIsPlayerBullet()) {
			Vector3 b = player->GetBulletPosition();

			bool isBulletHit = fabs(b.x - ePos.x) < bulletHitSize && fabs(b.y - ePos.y) < bulletHitSize;

			if (isBulletHit) {
				e->SetHP(0); // 敵を即死させる
			}
		}
	}


	particles->SetPlayerPos(player->GetPosition());
	particles->SetCameraPos(cameraController->GetTransform().translate);
	particles->SetGoalPos(goal->GetTranslate());
	particles->Update();

	field->Update();
	uimanager->SetPlayerHP(player->GetHP());
	uimanager->SetPlayerPosition({player->GetPosition().x, player->GetPosition().y});
	uimanager->Update();
	BG->Update(player->GetPosition());
	cameraController->SetTranslate({player->GetPosition().x, player->GetPosition().y + 5, cameraController->GetTransform().translate.z});
	cameraController->Update();
}

void GameScene::Draw() {

	GameBase::GetInstance()->ModelCommonSet();
	skyDome->Draw();
	player->Draw();

	enemyManager->Draw();

	field->Draw();
	goal->Draw(); 
	BG->Draw();
	ParticleManager::GetInstance()->Draw();
	
	GameBase::GetInstance()->SpriteCommonSet();
	uimanager->Draw();

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
