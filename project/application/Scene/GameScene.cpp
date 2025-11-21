#include "GameScene.h"
#include "ModelManeger.h"
#include "ParticleManager.h"
#include "Player/Player.h"
#include "Enemy/Enemy.h"
#include "CameraController.h"
#include "SkyDome.h"

GameScene::GameScene() {

	cameraController = new CameraController();
	
	ParticleManager::GetInstance()->CreateParticleGroup("test", "Resources/2d/uvChecker.png");
	particle = new ParticleEmitter("test", {0, 0, 0}, 1, 5, {0.01f, 0.01f, 0}, {0, 0, 0}, {5,10,1});
	skyDome = new SkyDome();
	player = new Player();
	enemy = new Enemy();
	field = new MapchipField();
	
}
GameScene::~GameScene(){
	Audio::GetInstance()->SoundUnload(&soundData);
	delete field;
	delete enemy;
	delete player;
	delete skyDome;
	delete particle;
	delete cameraController;
}

void GameScene::Initialize(GameBase* gameBase) {

	sceneEndClear = false;
	sceneEndOver = false;

	cameraController->Initialize();
	
	gameBase->SetDefaultCamera(cameraController->GetCamera());

	
	color = (uint8_t(meshColor.w * 255) << 24) | // A
	        (uint8_t(meshColor.x * 255) << 16) | // R
	        (uint8_t(meshColor.y * 255) << 8) |  // G
	        (uint8_t(meshColor.z * 255));        // B

	
	skyDome->Initialize(gameBase, cameraController->GetCamera());
	player->Initialize(gameBase,cameraController->GetCamera());
	enemy->Initialize(gameBase, cameraController->GetCamera());
	field->LoadFromCSV("Resources/CSV/MapChip_stage1.csv");
	field->Initialize(gameBase, cameraController->GetCamera());
	
	soundData = Audio::GetInstance()->SoundLoadFile("Resources/audio/Alarm01.wav");
	Audio::GetInstance()->SoundPlayWave(soundData);
}

void GameScene::Update(GameBase* gameBase) {

	
	skyDome->SetCamera(cameraController->GetCamera());
	player->SetCamera(cameraController->GetCamera());
	enemy->SetCamera(cameraController->GetCamera());
	field->SetCamera(cameraController->GetCamera());

	
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
	
	particle->Update({
	    {1, 1, 1},
        {0, 0, 0},
        {0, 0, 0}
    });
	

	ParticleManager::GetInstance()->Update(cameraController->GetCamera());
	skyDome->Update(gameBase);
	player->Update(gameBase);
	if (enemy->GetIsAlive()) {
	enemy->Update(gameBase);
	} else {
		sceneEndClear = true;
	}
	// ===== プレイヤーと敵の当たり判定 =====
	Vector3 p = player->GetPosition();
	Vector3 e = enemy->GetPosition();

	// 当たり判定サイズ（調整OK）
	float hitSize = 1.0f;

	// AABBチェック
	bool isColliding = fabs(p.x - e.x) < hitSize && fabs(p.y - e.y) < hitSize;

	if (!player->GetIsAlive()) {
		sceneEndOver = true;
	}

	// 衝突した
	if (isColliding) {

		// --- 上から踏んだ判定 ---
		bool isStomping = (p.y > e.y + 0.3f) &&          // プレイヤーが敵より上
		                  (player->GetVelocity().y < 0); // 落下中

		if (isStomping) {
			// 敵をひるませる
			enemy->Stun(); 
		} else {
			// それ以外はプレイヤーがダメージ
			player->Damage(1);
		}
	}
	// ===== プレイヤー弾と敵の当たり判定 =====
	if (player->GetIsPlayerBullet()&&enemy->GetIsAlive()) {
		// 弾の位置
		Vector3 b = player->GetBulletPosition();
		// 敵の位置
		Vector3 ePos = enemy->GetPosition();

		// 当たり判定のサイズ
		float bulletHitSize = 1.0f;

		bool isBulletHit = fabs(b.x - ePos.x) < bulletHitSize && fabs(b.y - ePos.y) < bulletHitSize;

		if (isBulletHit) {
			enemy->SetHP(0);
			
		}
	}

	field->Update();
	cameraController->SetTranslate({player->GetPosition().x, player->GetPosition().y + 5, cameraController->GetTransform().translate.z});
	cameraController->Update();
}

void GameScene::Draw(GameBase* gameBase) {

	gameBase->ModelCommonSet();
	skyDome->Draw();
	player->Draw(gameBase);
	if (enemy->GetIsAlive()){
	enemy->Draw(gameBase);
	}
	field->Draw(gameBase);

	//planeObject_->Draw();
	//axisObject_->Draw();
	ParticleManager::GetInstance()->Draw();
		

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
