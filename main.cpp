#include"GameBase.h"
#include "ResourceObject.h"
#include "DebugCamera.h"
#include "WaterController.h"
// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {

	CoInitializeEx(0, COINITBASE_MULTITHREADED);
	GameBase* gameBase=new GameBase;
	ResourceObject resourceObject;
	//エンジンの初期化
	gameBase->Initialize(L"CG2", 1280, 720);
	
	SetUnhandledExceptionFilter(gameBase->ExportDump);

	char keys[256]={0};
	char preKeys[256]={0};


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
	

	
	SoundData soundData1 = gameBase->SoundLoadWave("Resources/Alarm01.wav");

	gameBase->SoundPlayWave(soundData1);

	DebugCamera debugCamera;
	debugCamera.Initialize();
	Function function;
	WaterController water;
	water.Initialize();
	int PrePressMouse = 0;
	while (gameBase->IsMsgQuit()) {
		
		if (PeekMessage(gameBase->GetMsg(), NULL, 0, 0, PM_REMOVE)) {
			TranslateMessage(gameBase->GetMsg());
			DispatchMessage(gameBase->GetMsg());
		} else {

			gameBase->BeginFlame(keys,preKeys);
			memcpy(preKeys, keys, 256);
			LONG mx = gameBase->GetMouseX();
			LONG my = gameBase->GetMouseY();
			gameBase->Update();
			gameBase->UpdateMouse();
#pragma region 
				// DebugCamera の更新
			debugCamera.Update((uint8_t*)keys, (uint8_t*)preKeys);

			// DebugCameraの行列を取得
			Matrix4x4 viewProjectionMatrix = debugCamera.GetViewProjectionMatrix();

			// World行列を作る（必要に応じてGameBaseからTransformを使ってもよい）
			Matrix4x4 worldMatrix = function.MakeAffineMatrix(gameBase->transform.scale, gameBase->transform.rotate, gameBase->transform.translate);
			gameBase->GetCameraTransform() = {
			    {1, 1, 1},
                debugCamera.rotation_, debugCamera.translation_
            };
			// WVP行列を作成
			Matrix4x4 wvpMatrix = function.Multiply(worldMatrix, viewProjectionMatrix);

			// 書き込む
			*gameBase->GetTransformationMatrixData() = wvpMatrix;
#pragma endregion

		
			



			// 押し始め判定
			    bool justPressed = (gameBase->IsMousePressed(0) && PrePressMouse == 0);

			// チャージ処理
			if (gameBase->IsMousePressed(0)) {
				water.StartCharge({(float)mx, (float)my}, justPressed);
			}
			// 発射処理
			if (!gameBase->IsMousePressed(0) && PrePressMouse == 1) {
				water.Fire();
			}

			water.Update();
			water.Draw(*gameBase);

			//gameBase->DrawSpriteSheet(position, texcoord, 0xffffffff);
			//gameBase->DrawSpriteSheet(position2, texcoord, 0xffffffff);

			PrePressMouse = gameBase->IsMousePressed(0);
		

			/*gameBase->DrawSpriteSheet(position, texcoord, 0xffffffff);*/

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

