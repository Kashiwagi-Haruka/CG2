#include "GameScene.h"
#include "ModelManeger.h"
#include "ParticleManager.h"
#include "Player/Player.h"
#include "Enemy/Enemy.h"
#include "CameraController.h"
#include "SkyDome.h"

GameScene::GameScene() {

	spriteHandle = TextureManager::GetInstance()->GetTextureIndexByfilePath("Resources/2d/uvChecker.png");
	spriteHandle2 = TextureManager::GetInstance()->GetTextureIndexByfilePath("Resources/2d/monsterBall.png");
	ModelManeger::GetInstance()->LoadModel("plane");
	ModelManeger::GetInstance()->LoadModel("axis");
	sprite = new Sprite();
	sprite2_ = new Sprite();
	cameraController = new CameraController();
	planeObject_ = new Object3d();
	axisObject_ = new Object3d();
	ParticleManager::GetInstance()->CreateParticleGroup("test", "Resources/2d/uvChecker.png");
	particle = new ParticleEmitter("test", {0, 0, 0}, 1, 1);
	skyDome = new SkyDome();
	player = new Player();
	enemy = new Enemy();
	field = new MapchipField();
	field->LoadFromCSV("Resources/MapChip_stage1.csv");
}
GameScene::~GameScene(){

	delete field;
	
	delete enemy;
	delete player;
	delete skyDome;
	delete sprite;
	delete sprite2_;
	delete planeObject_;
	delete axisObject_;
	delete particle;
	delete cameraController;
}

void GameScene::Initialize(GameBase* gameBase) {

	sceneEnd = false;
	
	sprite->Initialize(gameBase->GetSpriteCommon(),spriteHandle);
	sprite2_->Initialize(gameBase->GetSpriteCommon(), spriteHandle2);

	cameraController->Initialize();
	
	gameBase->SetDefaultCamera(cameraController->GetCamera());
	
	axisObject_->Initialize(gameBase->GetObject3dCommon());
	planeObject_->Initialize(gameBase->GetObject3dCommon());
	
	planeObject_->SetModel("plane");
	axisObject_->SetModel("axis");
	
	color = (uint8_t(meshColor.w * 255) << 24) | // A
	        (uint8_t(meshColor.x * 255) << 16) | // R
	        (uint8_t(meshColor.y * 255) << 8) |  // G
	        (uint8_t(meshColor.z * 255));        // B

	
	spriteTexSize = {200,200};
	spriteTexSize2 = {200, 500};

	
	skyDome->Initialize(gameBase, cameraController->GetCamera());
	player->Initialize(gameBase,cameraController->GetCamera());
	enemy->Initialize(gameBase, cameraController->GetCamera());
	field->Initialize(gameBase, cameraController->GetCamera());

	// ★マップ参照を渡す
	player->SetMap(field);
}

void GameScene::Update(GameBase* gameBase) {

	cameraController->Update();
	skyDome->SetCamera(cameraController->GetCamera());
	player->SetCamera(cameraController->GetCamera());
	enemy->SetCamera(cameraController->GetCamera());
	field->Update();

	
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
	

	if (ImGui::Begin("Sprite Debug")) {
		ImGui::SetWindowSize(ImVec2(500, 100));
		ImGui::DragFloat3("Scale", &spriteTransform.scale.x, 1.0f, 0.1f, 9999.0f, "%4.1f");
		ImGui::DragFloat3("Rotation", &spriteTransform.rotate.x, 0.01f, 0.1f, 9999.0f, "%4.1f");
		ImGui::DragFloat3("Position", &spriteTransform.translate.x, 1.0f, 0.1f, 9999.0f, "%4.1f");
		ImGui::DragFloat2("TextureSize", &spriteTexSize.x, 1.0f);
	}
	ImGui::End();
	sprite->SetTextureRange({0.0f, 0.0f}, spriteTexSize);
	sprite->SetScale(spriteTransform.scale);
	sprite->SetRotation(spriteTransform.rotate);
	sprite->SetPosition(spriteTransform.translate);

	if (ImGui::Begin("Sprite2 Debug")) {

		ImGui::DragFloat3("Scale2", &sprite2Transform.scale.x, 1.0f, 0.1f, 10000.0f);
		ImGui::DragFloat3("Rotation2", &sprite2Transform.rotate.x, 0.01f);
		ImGui::DragFloat3("Position2", &sprite2Transform.translate.x, 1.0f);
		ImGui::DragFloat2("TextureSize2", &spriteTexSize2.x, 1.0f);
	}
	ImGui::End();

	/*if (ImGui::Begin("plane")) {
		ImGui::DragFloat3("planeScale", &planeTransform.scale.x, 1.0f, 0.1f, 10000.0f);
		ImGui::DragFloat3("planeRotation", &planeTransform.rotate.x, 0.01f);
		ImGui::DragFloat3("planePosition", &planeTransform.translate.x, 1.0f);
	}
	ImGui::End();*/
	planeObject_->SetScale(planeTransform.scale);
	planeObject_->SetRotate(planeTransform.rotate);
	planeObject_->SetTranslate(planeTransform.translate);
	if (ImGui::Begin("axis")) {
		ImGui::DragFloat3("axisScale", &axisTransform.scale.x, 1.0f, 0.1f, 10000.0f);
		ImGui::DragFloat3("axisRotation", &axisTransform.rotate.x, 0.01f);
		ImGui::DragFloat3("axisPosition", &axisTransform.translate.x, 1.0f);
	}
	ImGui::End();
	#endif
	axisObject_->SetScale(axisTransform.scale);
	axisObject_->SetRotate(axisTransform.rotate);
	axisObject_->SetTranslate(axisTransform.translate);

	sprite->Update();
	sprite2_->Update();
	planeObject_->Update();
	axisObject_->Update();
	particle->Update({
	    {1, 1, 1},
        {0, 0, 0},
        {0, 0, 0}
    });
	

	ParticleManager::GetInstance()->Update(cameraController->GetCamera());
	skyDome->Update(gameBase);
	player->Update(gameBase);
	enemy->Update(gameBase);
	cameraController->SetTranslate({player->GetPosition().x, player->GetPosition().y + 5, cameraController->GetTransform().translate.z});
	cameraController->Update();
}

void GameScene::Draw(GameBase* gameBase) {

	gameBase->ModelCommonSet();
	skyDome->Draw();
	player->Draw(gameBase);
	enemy->Draw(gameBase);
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

	


	

	gameBase->SpriteCommonSet();

	/*sprite->Draw();*/
	sprite2_->SetTextureRange({0.0f, 0.0f}, spriteTexSize2);
	sprite2_->SetScale(sprite2Transform.scale);
	sprite2_->SetRotation(sprite2Transform.rotate);
	sprite2_->SetPosition(sprite2Transform.translate);
	sprite2_->Draw();
	
	

}
