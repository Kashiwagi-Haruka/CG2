#include "GameScene.h"
#include "ModelManeger.h"
#include "ParticleManager.h"
#include "Player.h"
#include "Enemy.h"
GameScene::~GameScene(){

	delete player;
	delete sprite;
	delete sprite2_;
	delete planeObject_;
	delete axisObject_;
	delete particle;
}

void GameScene::Initialize(GameBase* gameBase) {

	uint32_t spriteHandle = TextureManager::GetInstance()->GetTextureIndexByfilePath("Resources/2d/uvChecker.png");
	uint32_t spriteHandle2 = TextureManager::GetInstance()->GetTextureIndexByfilePath("Resources/2d/monsterBall.png");

	sprite = new Sprite();
	sprite->Initialize(gameBase->GetSpriteCommon(),spriteHandle);
	sprite2_ = new Sprite();
	sprite2_->Initialize(gameBase->GetSpriteCommon(), spriteHandle2);


	camera = new Camera();
	camera->SetTranslate({0, 0, -50});
	gameBase->SetDefaultCamera(camera);
	planeObject_ = new Object3d();
	axisObject_ = new Object3d();
	ModelManeger::GetInstance()->LoadModel("plane");
	ModelManeger::GetInstance()->LoadModel("axis");
	axisObject_->Initialize(gameBase->GetObject3dCommon());
	planeObject_->Initialize(gameBase->GetObject3dCommon());
	planeObject_->SetModel("plane");
	axisObject_->SetModel("axis");
	ParticleManager::GetInstance()->CreateParticleGroup("test", "Resources/2d/uvChecker.png");

	particle = new ParticleEmitter("test", {0, 0, 0}, 10, 5);

	color = (uint8_t(meshColor.w * 255) << 24) | // A
	        (uint8_t(meshColor.x * 255) << 16) | // R
	        (uint8_t(meshColor.y * 255) << 8) |  // G
	        (uint8_t(meshColor.z * 255));        // B

	
	spriteTexSize = {200,200};
	spriteTexSize2 = {200, 500};


	player = new Player();
	player->Initialize(gameBase,camera);
	enemy = new Enemy();
	enemy->Initialize(gameBase, camera);
}

void GameScene::Update(GameBase* gameBase) {


	// WVP行列を作成
	
	Transform transforms[10];
	for (uint32_t index = 0; index < 10; ++index) {
		transforms[index].scale = {0.5f, 0.5f, 0.5f};
		transforms[index].rotate = {0.0f, 0.0f, 0.0f};
		transforms[index].translate = {index * 0.1f, index * 0.1f, index * 0.1f};
	}



	

	camera->Update();


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

	/*if (ImGui::Begin("Sprite Debug")) {

		ImGui::DragFloat3("Scale", &spriteTransform.scale.x, 1.0f, 0.1f, 10000.0f);
		ImGui::DragFloat3("Rotation", &spriteTransform.rotate.x, 0.01f);
		ImGui::DragFloat3("Position", &spriteTransform.translate.x, 1.0f);
		ImGui::DragFloat2("TextureSize", &spriteTexSize.x, 1.0f);
	}
	ImGui::End();*/
	sprite->SetTextureRange({0.0f, 0.0f}, spriteTexSize);
	sprite->SetScale(spriteTransform.scale);
	sprite->SetRotation(spriteTransform.rotate);
	sprite->SetPosition(spriteTransform.translate);

	/*if (ImGui::Begin("Sprite2 Debug")) {

		ImGui::DragFloat3("Scale2", &sprite2Transform.scale.x, 1.0f, 0.1f, 10000.0f);
		ImGui::DragFloat3("Rotation2", &sprite2Transform.rotate.x, 0.01f);
		ImGui::DragFloat3("Position2", &sprite2Transform.translate.x, 1.0f);
		ImGui::DragFloat2("TextureSize2", &spriteTexSize2.x, 1.0f);
	}
	ImGui::End();

	if (ImGui::Begin("plane")) {
		ImGui::DragFloat3("planeScale", &planeTransform.scale.x, 1.0f, 0.1f, 10000.0f);
		ImGui::DragFloat3("planeRotation", &planeTransform.rotate.x, 0.01f);
		ImGui::DragFloat3("planePosition", &planeTransform.translate.x, 1.0f);
	}
	ImGui::End();*/
	planeObject_->SetScale(planeTransform.scale);
	planeObject_->SetRotate(planeTransform.rotate);
	planeObject_->SetTranslate(planeTransform.translate);
	/*if (ImGui::Begin("axis")) {
		ImGui::DragFloat3("axisScale", &axisTransform.scale.x, 1.0f, 0.1f, 10000.0f);
		ImGui::DragFloat3("axisRotation", &axisTransform.rotate.x, 0.01f);
		ImGui::DragFloat3("axisPosition", &axisTransform.translate.x, 1.0f);
	}
	ImGui::End();*/
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
	ParticleManager::GetInstance()->Update(camera);
	player->Update(gameBase);
	enemy->Update(gameBase);
}

void GameScene::Draw(GameBase* gameBase) {

	gameBase->ModelCommonSet();
	player->Draw(gameBase);
	enemy->Draw(gameBase);
	
	//planeObject_->Draw();
	//axisObject_->Draw();
	/*gameBase->DrawParticle(modelData.vertices, color, ModelTextureHandle, ParticleWVPMatrix, ParticleWorldMatrix, 10);*/
	/*ParticleManager::GetInstance()->Draw();*/
		

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

	sprite->Draw();
	sprite2_->SetTextureRange({0.0f, 0.0f}, spriteTexSize2);
	sprite2_->SetScale(sprite2Transform.scale);
	sprite2_->SetRotation(sprite2Transform.rotate);
	sprite2_->SetPosition(sprite2Transform.translate);
	sprite2_->Draw();
	
	

}
