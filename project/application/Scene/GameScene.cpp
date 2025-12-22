#include "GameScene.h"
#include "CameraController.h"
#include "ModelManeger.h"
#include "Object/Background/SkyDome.h"
#include "Object/Enemy/EnemyManager.h"
#include "Object/Player/Player.h"
#include "Object3dCommon.h"
#include "ParticleManager.h"
#include "SceneManager.h"
#include <numbers>

GameScene::GameScene() {

	cameraController = std::make_unique<CameraController>();
	particles = std::make_unique<Particles>();
	skyDome = std::make_unique<SkyDome>();
	player = std::make_unique<Player>();
	enemyManager = std::make_unique<EnemyManager>();

	field = std::make_unique<MapchipField>();
	goal = std::make_unique<Goal>();
	sceneTransition = std::make_unique<SceneTransition>();
	uimanager = std::make_unique<UIManager>();
	/*BG = std::make_unique<Background>();*/
	house = std::make_unique<House>();

	BGMData = Audio::GetInstance()->SoundLoadFile("Resources/audio/昼下がり気分.mp3");
}

GameScene::~GameScene() {}

void GameScene::Finalize() {

	Audio::GetInstance()->SoundUnload(&BGMData);

	// level up icons
	for (int i = 0; i < 4; i++) {
		levelupIcons[i].reset();
	}
}

void GameScene::Initialize() {

	sceneEndClear = false;
	sceneEndOver = false;

	cameraController->Initialize();

	GameBase::GetInstance()->SetDefaultCamera(cameraController->GetCamera());

	skyDome->Initialize(cameraController->GetCamera());
	player->Initialize(cameraController->GetCamera());

	enemyManager->Initialize(cameraController->GetCamera());
	field->LoadFromCSV("Resources/CSV/MapChip_stage1.csv");
	field->Initialize(cameraController->GetCamera());
	goal->Initialize(cameraController->GetCamera());
	sceneTransition->Initialize();
	uimanager->SetPlayerHPMax(player->GetHPMax());
	uimanager->SetPlayerHP(player->GetHP());

	uimanager->Initialize();
	/*BG->SetCamera(cameraController->GetCamera());
	BG->SetPosition(player->GetPosition());
	BG->Initialize();*/
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

		levelupIcons[i] = std::make_unique<Sprite>();
		levelupIcons[i]->Initialize(GameBase::GetInstance()->GetSpriteCommon(), handle[i]);
		levelupIcons[i]->SetScale({256, 256});
	}

	Audio::GetInstance()->SoundPlayWave(BGMData);
	pointLight_.color = {1.0f, 1.0f, 1.0f, 1.0f};
	pointLight_.position = {0.0f, 5.0f, 0.0f};
	pointLight_.intensity = 0.0f;
	pointLight_.radius = 10.0f;
	pointLight_.decay = 1.0f;

	directionalLight_.color = {1.0f, 1.0f, 1.0f, 1.0f};
	directionalLight_.direction = {0.0f, -1.0f, 0.5f};
	directionalLight_.intensity = 1.0f;

	spotLight_.color = {1.0f, 1.0f, 1.0f, 1.0f};
	spotLight_.position = {2.0f, 1.25f, 0.0f};
	spotLight_.direction = {-1.0f, -1.0f, 0.0f};
	spotLight_.intensity = 0.0f;
	spotLight_.distance = 7.0f;
	spotLight_.decay = 2.0f;
	spotLight_.cosAngle = std::cos(std::numbers::pi_v<float> / 3.0f);
	spotLight_.cosFalloffStart = std::cos(std::numbers::pi_v<float> / 4.0f);
}

void GameScene::DebugImGui() {

#ifdef USE_IMGUI
	if (ImGui::Begin("SampleLight")) {
		if (ImGui::TreeNode("DirectionalLight")) {
			ImGui::ColorEdit4("LightColor", &directionalLight_.color.x);
			ImGui::DragFloat3("LightDirection", &directionalLight_.direction.x, 0.1f, -1.0f, 1.0f);
			ImGui::DragFloat("LightIntensity", &directionalLight_.intensity, 0.1f, 0.0f, 10.0f);
			ImGui::TreePop();
		}
		if (ImGui::TreeNode("PointLight")) {
			ImGui::ColorEdit4("PointLightColor", &pointLight_.color.x);
			ImGui::DragFloat("PointLightIntensity", &pointLight_.intensity, 0.1f);
			ImGui::DragFloat3("PointLightPosition", &pointLight_.position.x, 0.1f);
			ImGui::DragFloat("PointLightRadius", &pointLight_.radius, 0.1f);
			ImGui::DragFloat("PointLightDecay", &pointLight_.decay, 0.1f);
			ImGui::TreePop();
		}
		if (ImGui::TreeNode("SpotLight")) {
			ImGui::ColorEdit4("SpotLightColor", &spotLight_.color.x);
			ImGui::DragFloat("SpotLightIntensity", &spotLight_.intensity, 0.1f);
			ImGui::DragFloat3("SpotLightPosition", &spotLight_.position.x, 0.1f);
			ImGui::DragFloat3("SpotLightDirection", &spotLight_.direction.x, 0.1f);
			ImGui::DragFloat("SpotLightDistance", &spotLight_.distance, 0.1f);
			ImGui::DragFloat("SpotLightDecay", &spotLight_.decay, 0.1f);
			ImGui::DragFloat("SpotLightCosAngle", &spotLight_.cosAngle, 0.1f, 0.0f, 1.0f);
			ImGui::DragFloat("SpotLightCosFalloffStart", &spotLight_.cosFalloffStart, 0.1f, 0.0f, 1.0f);
			ImGui::TreePop();
		}
		ImGui::End();
	}

	// ★ ウェーブ情報の表示
	if (ImGui::Begin("Wave Info")) {
		ImGui::Text("Current Wave: %d", enemyManager->GetCurrentWave());
		ImGui::Text("Alive Enemies: %d", enemyManager->GetAliveEnemyCount());
		ImGui::Text("Total Killed: %d", enemyManager->GetTotalEnemiesKilled());
		ImGui::ProgressBar(enemyManager->GetWaveProgress(), ImVec2(0.0f, 0.0f));

		const char* stateNames[] = {"Waiting", "Spawning", "Active", "Complete"};
		int stateIndex = static_cast<int>(enemyManager->GetWaveState());
		ImGui::Text("Wave State: %s", stateNames[stateIndex]);

		float waveDelay = 3.0f;
		if (ImGui::DragFloat("Wave Delay", &waveDelay, 0.1f, 0.5f, 10.0f)) {
			enemyManager->SetWaveDelay(waveDelay);
		}
		ImGui::End();
	}

#endif // USE_IMGUI
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

	DebugImGui();
	GameBase::GetInstance()->GetObject3dCommon()->SetDirectionalLight(directionalLight_);
	GameBase::GetInstance()->GetObject3dCommon()->SetPointLight(pointLight_);
	GameBase::GetInstance()->GetObject3dCommon()->SetSpotLight(spotLight_);
	skyDome->SetCamera(cameraController->GetCamera());
	player->SetCamera(cameraController->GetCamera());
	field->SetCamera(cameraController->GetCamera());
	goal->SetCamera(cameraController->GetCamera());
	/*BG->SetCamera(cameraController->GetCamera());*/

	ParticleManager::GetInstance()->Update(cameraController->GetCamera());
	skyDome->Update();

	player->Update();
	// ===========================
	// ★ レベルアップを検知して選択画面へ
	// ===========================
	if (player->GetLv() && !isLevelSelecting) {

		isLevelSelecting = true;

		// ランダムで 0～3 から2個選ぶ
		int a = rand() % 4;
		int b = rand() % 4;
		while (b == a) {
			b = rand() % 4;
		}

		selectChoices[0] = a;
		selectChoices[1] = b;

		cursorIndex = 0; // 左から開始

		// プレイヤーの動きを停止させたい場合はここで何もしない(GameScene が制御)
	}

	// ★ ウェーブシステムの更新
	enemyManager->Update(cameraController->GetCamera());

	// ★ ウェーブクリア判定でゴールを有効化
	// 全ウェーブクリア後にゴールを出すなど、条件は自由に変更可能
	if (enemyManager->GetCurrentWave() >= 5 && enemyManager->IsWaveComplete()) {
		goalActive = true;
	}

	goal->Update();

	// 当たり判定サイズ(調整OK)
	float hitSize = 1.0f;
	// ===== プレイヤーとゴールの当たり判定 =====
	{
		Vector3 p = player->GetPosition();
		Vector3 g = goal->GetTranslate();

		float goalHitSize = 2.0f;

		if (goalActive) { // ★ 条件クリア後だけ処理しない
			bool isGoalHit = fabs(p.x - g.x) < goalHitSize && fabs(p.y - g.y) < goalHitSize;

			if (isGoalHit) {

				SceneManager::GetInstance()->ChangeScene("Result");
			}
		}
	}

	if (!player->GetIsAlive()) {

		SceneManager::GetInstance()->ChangeScene("GameOver");
	}

	// ===== プレイヤーと敵の当たり判定 =====
	Vector3 p = player->GetPosition();
	Vector3 v = player->GetVelocity();

	Vector3 housePos = house->GetPosition();
	float houseHitSize = 8.0f;

	for (auto& e : enemyManager->GetEnemies()) {

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
				e->Stun(); // 敵にダメージ(スタン)
			} else {
				player->Damage(1); // プレイヤーが被弾
			}
		}

		// ===== ② 剣との当たり判定 =====
		if (player->GetIsAlive() && player->GetSword()->IsAttacking()) {

			Vector3 swordPos = player->GetSword()->GetPosition();
			float swordHit = player->GetSword()->GetHitSize();

			bool hitSword = fabs(swordPos.x - ePos.x) < swordHit && fabs(swordPos.y - ePos.y) < swordHit;

			if (hitSword) {
				e->SetHPSubtract(1); // ダメージ

				// 敵を倒したらEXP獲得
				if (!e->GetIsAlive()) {
					player->EXPMath();
				}
			}
		}

		// ===== ③ House と敵の当たり判定 =====
		bool hitHouse = fabs(ePos.x - housePos.x) < houseHitSize && fabs(ePos.y - housePos.y) < houseHitSize;

		if (hitHouse) {
			e->SetHPSubtract(10); // ★ 敵を即死させる or 消す処理
			house->Damage(1);     // ★ house にダメージ

			if (house->GetHP() <= 0) {
				SceneManager::GetInstance()->ChangeScene("GameOver");
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
	uimanager->SetHouseHP(house->GetHP());
	uimanager->SetHouseHPMax(30); // House の最大HP(好きに変更)

	uimanager->Update();
	/*BG->Update(player->GetPosition());*/
	house->Update(cameraController->GetCamera());

	// どちらかの HP が 0 以下で GameOver
	if (!player->GetIsAlive() || house->GetHP() <= 0) {
		sceneEndOver = true;
	}

	cameraController->SetPlayerPos(player->GetPosition());

	cameraController->Update();
}

void GameScene::Draw() {

	GameBase::GetInstance()->ModelCommonSet();
	skyDome->Draw();
	player->Draw();
	enemyManager->Draw();
	house->Draw();

	field->Draw();
	if (goalActive) {
		goal->Draw(); // ★ 条件クリア後だけ描画する
	}

	/*BG->Draw();*/
	ParticleManager::GetInstance()->Draw();

	GameBase::GetInstance()->SpriteCommonSet();
	uimanager->Draw();
	// =============================
	// ★ レベルアップ選択画面描画(別スプライト使用版)
	// =============================
	if (isLevelSelecting) {

		GameBase::GetInstance()->SpriteCommonSet();

		// 左と右の選択肢
		int leftID = selectChoices[0];
		int rightID = selectChoices[1];

		// 表示位置(自由に調整可)
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
}