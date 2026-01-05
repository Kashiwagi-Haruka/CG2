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
#include "RigidBody.h"
#include "GameTimer.h"

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

	BGMData = Audio::GetInstance()->SoundLoadFile("Resources/audio/BGM/Tailshaft.mp3");
	GameTimer::GetInstance()->Reset();
	GameBase::GetInstance()->SetIsCursorStablity(true);
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
	isBGMPlaying = false;
	cameraController->Initialize();

	GameBase::GetInstance()->SetDefaultCamera(cameraController->GetCamera());

	skyDome->Initialize(cameraController->GetCamera());
	player->Initialize(cameraController->GetCamera());

	enemyManager->Initialize(cameraController->GetCamera());
	field->LoadFromCSV("Resources/CSV/MapChip_stage1.csv");
	field->Initialize(cameraController->GetCamera());
	goal->Initialize(cameraController->GetCamera());
	sceneTransition->Initialize(false);
	isTransitionIn = true;
	isTransitionOut = false;
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

	
	pointLight_.color = {1.0f, 1.0f, 1.0f, 1.0f};
	pointLight_.position = {-25.0f, 5.0f, -25.0f};
	pointLight_.intensity = 1.0f;
	pointLight_.radius = 20.0f;
	pointLight_.decay = 0.7f;

	directionalLight_.color = {0.3725f, 0.2667f, 0.7882f,1.0f};
	directionalLight_.direction = {0.0f, -1.0f, 0.5f};
	directionalLight_.intensity = 1.0f;

	spotLight_.color = {1.0f, 1.0f, 1.0f, 1.0f};
	spotLight_.position = {-50.0f, 5.0f, -50.0f};
	spotLight_.direction = {0.0f, 1.0f, 0.0f};
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
		
	}
	ImGui::End();
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
	GameTimer::GetInstance()->Update();
	if (!isBGMPlaying) {
		Audio::GetInstance()->SoundPlayWave(BGMData, true);
		isBGMPlaying = true;
	}
	auto makeAabb = [](const Vector3& center, const Vector3& halfSize) {
		AABB aabb;
		aabb.min = {center.x - halfSize.x, center.y - halfSize.y, center.z - halfSize.z};
		aabb.max = {center.x + halfSize.x, center.y + halfSize.y, center.z + halfSize.z};
		return aabb;
	};

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
	

	ParticleManager::GetInstance()->Update(cameraController->GetCamera());
	skyDome->Update();
	field->Update();
	house->Update(cameraController->GetCamera());
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
	enemyManager->Update(cameraController->GetCamera(), house->GetPosition(), player->GetPosition(), player->GetIsAlive());

	// ★ ウェーブクリア判定でゴールを有効化
	// 全ウェーブクリア後にゴールを出すなど、条件は自由に変更可能
	if (enemyManager->GetCurrentWave() >= 5 && enemyManager->IsWaveComplete()) {
		goalActive = true;
	}

	goal->Update();


	// ===== プレイヤーとゴールの当たり判定 =====
	{
		Vector3 p = player->GetPosition();
		Vector3 g = goal->GetTranslate();

		float goalHitSize = 2.0f;

		if (goalActive) { // ★ 条件クリア後だけ処理しない
			AABB playerAabb = makeAabb(p, player->GetScale());
			AABB goalAabb = makeAabb(g, {goalHitSize, goalHitSize, goalHitSize});
			bool isGoalHit = RigidBody::isCollision(playerAabb, goalAabb);

			if (isGoalHit) {

				SceneManager::GetInstance()->ChangeScene("Result");
			}
		}
	}

	if (!player->GetIsAlive()) {
		if (!isTransitionOut) {
			isTransitionOut = true;
		}
		
	}

	// ===== プレイヤーと敵の当たり判定 =====
	Vector3 p = player->GetPosition();
	Vector3 v = player->GetVelocity();
	AABB playerAabb = makeAabb(p, player->GetScale());

	Vector3 housePos = house->GetPosition();
	AABB houseAabb = makeAabb(housePos, house->GetScale());

	for (auto& e : enemyManager->GetEnemies()) {

		if (!e->GetIsAlive())
			continue; // 死んだ敵はスキップ

		Vector3 ePos = e->GetPosition();
		AABB enemyAabb = makeAabb(ePos, e->GetScale());
		bool hitHouseBody = RigidBody::isCollision(enemyAabb, houseAabb);
		if (hitHouseBody) {
			Vector3 toEnemy = ePos - housePos;
			toEnemy.y = 0.0f;
			if (LengthSquared(toEnemy) < 0.0001f) {
				toEnemy = {1.0f, 0.0f, 0.0f};
			}
			Vector3 pushDir = Function::Normalize(toEnemy);
			Vector3 houseScale = house->GetScale();
			Vector3 enemyScale = e->GetScale();
			float minDistance = houseScale.x + enemyScale.x;
			Vector3 correctedPos = housePos + pushDir * minDistance;
			correctedPos.y = ePos.y;
			e->SetPosition(correctedPos);
			ePos = correctedPos;
			enemyAabb = makeAabb(ePos, e->GetScale());
		}
		// ===== ① 剣との当たり判定 =====
		if (player->GetIsAlive() && player->GetSword()->IsAttacking()) {

			Vector3 swordPos = player->GetSword()->GetPosition();
			float swordHit = player->GetSword()->GetHitSize();

			AABB swordAabb = makeAabb(swordPos, {swordHit, swordHit, swordHit});
			bool hitSword = RigidBody::isCollision(swordAabb, enemyAabb);

			if (hitSword) {
				e->SetHPSubtract(1); // ダメージ
				enemyManager->OnEnemyDamaged(e.get());
				// 敵を倒したらEXP獲得
				if (!e->GetIsAlive()) {
					player->EXPMath();
				}
			}
		}
		// ===== ② スキル攻撃との当たり判定 =====
		if (player->GetIsAlive() && player->GetSkill() && player->GetSkill()->IsDamaging()) {
			AABB skillAabb = makeAabb(player->GetSkill()->GetDamagePosition(), player->GetSkill()->GetDamageScale());
			bool hitSkill = RigidBody::isCollision(skillAabb, enemyAabb);

			if (hitSkill) {
				e->SetHPSubtract(1);
				enemyManager->OnEnemyDamaged(e.get());
				if (!e->GetIsAlive()) {
					player->EXPMath();
				}
			}
		}

		// ===== ③ 敵の攻撃判定 =====
		if (e->IsAttackHitActive()) {
			AABB enemyAttackAabb = makeAabb(e->GetAttackPosition(), {e->GetAttackHitSize(), e->GetAttackHitSize(), e->GetAttackHitSize()});
			bool hitEnemyAttack = RigidBody::isCollision(enemyAttackAabb, playerAabb);
			if (hitEnemyAttack) {
				player->Damage(1);
			}
			bool hitHouseAttack = RigidBody::isCollision(enemyAttackAabb, houseAabb);
			if (hitHouseAttack) {
				house->Damage(1);
			}
		}

		// ===== ③ House と敵の当たり判定 =====
	
	}




	uimanager->SetPlayerParameters(player->GetParameters());
	uimanager->SetPlayerHP(player->GetHP());
	uimanager->SetHouseHP(house->GetHP());
	

	uimanager->Update();
	
	
	particles->SetCameraPos(cameraController->GetCamera()->GetTranslate());
	particles->SetPlayerPos(player->GetPosition());
	particles->SetGoalPos(goal->GetTranslate());
	particles->Update();

	cameraController->SetPlayerPos(player->GetPosition());

	cameraController->Update();

	if (isTransitionIn||isTransitionOut) {
		sceneTransition->Update();
		if (sceneTransition->IsEnd()&&isTransitionOut) {
			SceneManager::GetInstance()->ChangeScene("GameOver");
		}
	}
}

void GameScene::Draw() {

	GameBase::GetInstance()->ModelCommonSet();
	skyDome->Draw();
	field->Draw();
	house->Draw();
	player->Draw();
	enemyManager->Draw();
	




	particles->Draw();
	

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
	if (isTransitionIn || isTransitionOut) {
		sceneTransition->Draw();
	}
}