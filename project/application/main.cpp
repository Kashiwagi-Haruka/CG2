#include"GameBase.h"
#include "ResourceObject.h"
#include "DebugCamera.h"


// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {

	GameBase* gameBase=new GameBase;
	ResourceObject resourceObject;
	//エンジンの初期化
	gameBase->Initialize(L"CG2", 1280, 720);

	

		
	ModelData modelData = gameBase->LoadObjFile("Resources/3d", "plane.obj");
	int textureHandle = gameBase->LoadTextures("Resources/2d/uvChecker.png");

	/*ModelData fenceModel = gameBase->LoadObjFile("Resources/3d", "fence.obj");
	int fenceHandle = gameBase->LoadTextures("Resources/3d/fence.png");*/


	SetUnhandledExceptionFilter(gameBase->ExportDump);

	char keys[256]={0};
	char preKeys[256]={0};

	#pragma region c
	Vector3 position[4]{
	    {100, 100, 0}, // 左上 (100,100)
	    {300, 100, 0}, // 右上
	    {300, 300, 0}, // 右下
	    {100, 300, 0}  // 左下
	};
	Vector3 position2[4]{
	    {200, 200, 0}, // 左上 (100,100)
	    {400, 200, 0}, // 右上
	    {400, 400, 0}, // 右下
	    {200, 400, 0}  // 左下
	};
	Vector2 texcoord[4] = {
	    {0.0f, 0.0f},
        {1.0f, 0.0f},
        {1.0f, 1.0f},
        {0.0f, 1.0f}
    };

	SoundData soundData1 = gameBase->SoundLoadWave("Resources/audio/Alarm01.wav");

	gameBase->SoundPlayWave(soundData1);

	DebugCamera camera;
	camera.Initialize();
	Function function;
#pragma endregion

	
	enum class SceneName{

		Title,
		Game,

	};

	SceneName scene=SceneName::Title;

	#ifdef NDEBUG

		scene = SceneName::Game;

	#endif // DEBUG

	
		Transform planeTransform{
	        .scale{1, 1, 1},
            .rotate{0, 0, 0},
            .translate{0, 0, 0}
        };

		Transform fenceTransform{
	        .scale{1, 1, 1},
            .rotate{0, 0, 0},
            .translate{0, 0, 0}
        };
	    
		// ループの外側で保持する色変数
	    static ImVec4 meshColor = ImVec4(1.0f, 1.0f, 1.0f, 1.0f); // 初期値: 白


		DirectionalLight light{
	        .color{1, 1, 1, 1},
            .direction{0, 0, 1},
            .intensity{1}
        };
		
		bool IsPKey = false;
	    bool IsXButton = false;
	    bool IsKeyboard = true;

		/*gameBase->SetBlendMode(kBlendModeNone);*/


		
	/*int PrePressMouse = 0;*/
	while (gameBase->ProcessMessage()) {
		

		gameBase->BeginFlame();
			

#pragma region c
			
				
		camera.Update((uint8_t*)keys, (uint8_t*)preKeys);

			
		Matrix4x4 viewProjectionMatrix = camera.GetViewProjectionMatrix();

		// World行列を作る（必要に応じてGameBaseからTransformを使ってもよい）
		Matrix4x4 worldMatrix = function.MakeAffineMatrix(planeTransform.scale, planeTransform.rotate, planeTransform.translate);
		Matrix4x4 fenceWorldMatrix = function.MakeAffineMatrix(fenceTransform.scale, fenceTransform.rotate, fenceTransform.translate);
		gameBase->GetCameraTransform() = {
		    {1, 1, 1},
            camera.rotation_, camera.translation_
        };
		// WVP行列を作成
		Matrix4x4 wvpMatrix = function.Multiply(worldMatrix, viewProjectionMatrix);
		Matrix4x4 fenceWvpMatrix = function.Multiply(fenceWorldMatrix, viewProjectionMatrix);


			
#pragma endregion


		switch (scene) {

		case SceneName::Title:

				

			break;

		case SceneName::Game:

				
			break;
		default:
			break;
		}
		
		ImGui::Begin("Plane");
		ImGui::Text("Transform");
		ImGui::DragFloat3("Scale", &planeTransform.scale.x,0.01f);
		ImGui::DragFloat3("Rotate", &planeTransform.rotate.x,0.01f);
		ImGui::DragFloat3("Translate", &planeTransform.translate.x,0.01f);
		ImGui::ColorEdit4("Color", (float*)&meshColor);
		
		if (ImGui::CollapsingHeader("light Settings")) {
			// 方向
			ImGui::DragFloat3("light Direction", &light.direction.x, 0.1f, -1.0f, 1.0f);
			// 明るさ
			ImGui::SliderFloat("light Intensity", &light.intensity, 0.0f, 5.0f);
			// 色
			ImGui::ColorEdit3("light Color", &light.color.x);
		}

		gameBase->SetDirectionalLightData(light);
		
		// --- ブレンドモード選択 ---
		static int blendModeIndex = 0;
		const char* blendModes[] = {"None", "Alpha", "Add", "Sub", "Mul", "Screen"};
		if (ImGui::Combo("Blend Mode", &blendModeIndex, blendModes, IM_ARRAYSIZE(blendModes))) {
			gameBase->SetBlendMode(static_cast<BlendMode>(blendModeIndex));
		}

		ImGui::End();

		worldMatrix = function.MakeAffineMatrix(planeTransform.scale, planeTransform.rotate, planeTransform.translate);

		

		ImGui::Begin("PadorKey");
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
			


		ImGui::End();

		uint32_t color = (uint8_t(meshColor.w * 255) << 24) | // A
		                 (uint8_t(meshColor.x * 255) << 16) | // R
		                 (uint8_t(meshColor.y * 255) << 8) |  // G
		                 (uint8_t(meshColor.z * 255));        // B

		

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

					if (gameBase->PushButton(Input::PadButton::kButtonUp)|| gameBase->PushButton(Input::PadButton::kButtonDown) || gameBase->PushButton(Input::PadButton::kButtonLeft) || gameBase->PushButton(Input::PadButton::kButtonRight)) {
						if (gameBase->PushButton(Input::PadButton::kButtonUp)) {
							planeTransform.translate.y += 0.1f;
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
			

		gameBase->DrawParticle(modelData.vertices, color, textureHandle, wvpMatrix, worldMatrix, 10);
			
		
			//ゲームの処理

			gameBase->EndFlame();
		
	}

	//出力ウィンドウへの文字出力
	gameBase->SoundUnload(&soundData1);
	delete gameBase;
	resourceObject.LeakChecker();
	CoUninitialize();

	return 0;

}

