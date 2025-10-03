#include"GameBase.h"
#include "ResourceObject.h"
#include "DebugCamera.h"


// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {

	CoInitializeEx(0, COINITBASE_MULTITHREADED);
	GameBase* gameBase=new GameBase;
	ResourceObject resourceObject;
	//エンジンの初期化
	gameBase->Initialize(L"CG2", 1280, 720);

	gameBase->CreateResource();

		
	ModelData modelData = gameBase->LoadObjFile("Resources/3d", "plane.obj");
	int textureHandle = gameBase->LoadTextures("Resources/2d/uvChecker.png");

	

	


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

	


	
	/*int PrePressMouse = 0;*/
	while (gameBase->IsMsgQuit()) {
		
		if (PeekMessage(gameBase->GetMsg(), NULL, 0, 0, PM_REMOVE)) {
			TranslateMessage(gameBase->GetMsg());
			DispatchMessage(gameBase->GetMsg());
		} else {

			gameBase->BeginFlame(keys,preKeys);
			memcpy(preKeys, keys, 256);

#pragma region c
			
				
			camera.Update((uint8_t*)keys, (uint8_t*)preKeys);

			
			Matrix4x4 viewProjectionMatrix = camera.GetViewProjectionMatrix();

			// World行列を作る（必要に応じてGameBaseからTransformを使ってもよい）
			Matrix4x4 worldMatrix = function.MakeAffineMatrix(gameBase->transform.scale, gameBase->transform.rotate, gameBase->transform.translate);
			gameBase->GetCameraTransform() = {
			    {1, 1, 1},
                camera.rotation_, camera.translation_
            };
			// WVP行列を作成
			Matrix4x4 wvpMatrix = function.Multiply(worldMatrix, viewProjectionMatrix);

			// 書き込む
			//gameBase->GetTransformationMatrixData()[0].WVP = wvpMatrix;
			//gameBase->GetTransformationMatrixData()[1].WVP = wvpMatrix;
			gameBase->SetTransformMatrixWVP(viewProjectionMatrix, 0);
			gameBase->SetTransformMatrixWVP(viewProjectionMatrix, 1);
			
#pragma endregion

	

			

			switch (scene) {

			case SceneName::Title:

				

				break;

			case SceneName::Game:

				
				break;
			default:
				break;
			}
		
			Transform planeTransform{
			    .scale{1, 1, 1},
                .rotate{0, 0, 0},
                .translate{0, 0, 0}
			};
			Vector4 Color{1, 1, 1, 1};
		
			gameBase->DrawMesh(modelData.vertices, 0xffffffff, textureHandle, wvpMatrix, worldMatrix);
			
			ImGui::Begin("Plane");
			ImGui::Text("Transform");
			ImGui::DragFloat3("Scale", &planeTransform.scale.x);
			ImGui::DragFloat3("Rotate", &planeTransform.rotate.x);
			ImGui::DragFloat3("Translate", &planeTransform.translate.x);
			ImGui::ColorEdit4("Color", &Color.x);
			
			

			ImGui::End();

		
	

			//ゲームの処理

			gameBase->EndFlame();
		}



	}

	//出力ウィンドウへの文字出力
	gameBase->OutPutLog();
	gameBase->SoundUnload(&soundData1);
	delete gameBase;
	resourceObject.LeakChecker();
	CoUninitialize();

	return 0;

}

