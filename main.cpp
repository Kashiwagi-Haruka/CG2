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
	
	SetUnhandledExceptionFilter(gameBase->ExportDump);

	char keys[256]={0};
	char preKeys[256]={0};


	Vector3 position[3]{
		{-20,0,0,},
		{20,0,0},
		{0,20,0}
	};
	Vector2 texcoord[3] = {{0.0f, 1.0f}, {0.5f, 0.0f},{1.0f, 1.0f}};
	
	SoundData soundData1 = gameBase->SoundLoadWave("Resources/Alarm01.wav");

	gameBase->SoundPlayWave(soundData1);

	DebugCamera debugCamera;
	debugCamera.Initialize();
	Function function;
	while (gameBase->IsMsgQuit()) {
		
		if (PeekMessage(gameBase->GetMsg(), NULL, 0, 0, PM_REMOVE)) {
			TranslateMessage(gameBase->GetMsg());
			DispatchMessage(gameBase->GetMsg());
		} else {

			gameBase->BeginFlame(keys,preKeys);
			memcpy(preKeys, keys, 256);

			gameBase->Update();
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

