#include "GameScene.h"

GameScene::~GameScene(){

	delete sprite;
	delete sprite2_;

}

void GameScene::Initialize(GameBase* gameBase) {

	uint32_t spriteHandle = TextureManager::GetInstance()->GetTextureIndexByfilePath("Resources/2d/uvChecker.png");
	uint32_t spriteHandle2 = TextureManager::GetInstance()->GetTextureIndexByfilePath("Resources/2d/monsterBall.png");

	sprite = new Sprite();
	sprite->Initialize(gameBase->GetSpriteCommon(),spriteHandle);
	sprite2_ = new Sprite();
	sprite2_->Initialize(gameBase->GetSpriteCommon(), spriteHandle2);

	camera.Initialize();



	color = (uint8_t(meshColor.w * 255) << 24) | // A
	        (uint8_t(meshColor.x * 255) << 16) | // R
	        (uint8_t(meshColor.y * 255) << 8) |  // G
	        (uint8_t(meshColor.z * 255));        // B

	modelData = gameBase->LoadObjFile("Resources/3d", "plane.obj");
	spriteTexSize = {200,200};
	spriteTexSize2 = {200, 500};

}

void GameScene::Update(GameBase* gameBase) {

	viewProjectionMatrix = camera.GetViewProjectionMatrix();

	// World行列を作る（必要に応じてGameBaseからTransformを使ってもよい）
	worldMatrix = Function::MakeAffineMatrix(planeTransform.scale, planeTransform.rotate, planeTransform.translate);
	fenceWorldMatrix = Function::MakeAffineMatrix(fenceTransform.scale, fenceTransform.rotate, fenceTransform.translate);
	gameBase->GetCameraTransform() = {
	    {1, 1, 1},
        camera.rotation_, camera.translation_
    };
	// WVP行列を作成
	wvpMatrix = Function::Multiply(worldMatrix, viewProjectionMatrix);
	Transform transforms[10];
	for (uint32_t index = 0; index < 10; ++index) {
		transforms[index].scale = {0.5f, 0.5f, 0.5f};
		transforms[index].rotate = {0.0f, 0.0f, 0.0f};
		transforms[index].translate = {index * 0.1f, index * 0.1f, index * 0.1f};
	}

	for (uint32_t index = 0; index < 10; ++index) {
	
		ParticleWorldMatrix = Function::MakeAffineMatrix(transforms[index].scale, transforms[index].rotate, transforms[index].translate);
		ParticleWVPMatrix = Function::Multiply(ParticleWorldMatrix, ParticleWVPMatrix);

	}

	fenceWvpMatrix = Function::Multiply(fenceWorldMatrix, viewProjectionMatrix);

	camera.Update((uint8_t*)keys, (uint8_t*)preKeys);


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

	worldMatrix = Function::MakeAffineMatrix(planeTransform.scale, planeTransform.rotate, planeTransform.translate);

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
}

void GameScene::Draw(GameBase* gameBase) {




		

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

	/*gameBase->DrawParticle(modelData.vertices, color, textureHandle, ParticleWVPMatrix, ParticleWorldMatrix, 10);*/

	if (ImGui::Begin("Sprite Debug")) {
		    
		ImGui::DragFloat3("Scale", &spriteTransform.scale.x, 1.0f, 0.1f, 10000.0f);
		ImGui::DragFloat3("Rotation", &spriteTransform.rotate.x, 0.01f);
		ImGui::DragFloat3("Position", &spriteTransform.translate.x, 1.0f);
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
	
		sprite->Update();

	gameBase->SpriteCommonSet();

	sprite->Draw();
	sprite2_->SetTextureRange({0.0f, 0.0f}, spriteTexSize2);
	sprite2_->SetScale(sprite2Transform.scale);
	sprite2_->SetRotation(sprite2Transform.rotate);
	sprite2_->SetPosition(sprite2Transform.translate);

	sprite2_->Update();

	sprite2_->Draw();
}
