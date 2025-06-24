#include"GameBase.h"
#include "ResourceObject.h"
// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {

	CoInitializeEx(0, COINITBASE_MULTITHREADED);
	GameBase* gameBase=new GameBase;
	ResourceObject resourceObject;
	//エンジンの初期化
	gameBase->Initialize(L"CG2", 1280, 720);
	
	SetUnhandledExceptionFilter(gameBase->ExportDump);

	Vector3 position[3]{
		{-20,0,0,},
		{20,0,0},
		{0,20,0}
	};
	Vector2 texcoord[3] = {{0.0f, 1.0f}, {0.5f, 0.0f},{1.0f, 1.0f}};
	
	SoundData soundData1 = gameBase->SoundLoadWave("Resources/Alarm01.wav");

	gameBase->SoundPlayWave(soundData1);
	
	while (gameBase->IsMsgQuit()) {
		memcpy(preKey, key, sizeof(key));
		if (PeekMessage(gameBase->GetMsg(), NULL, 0, 0, PM_REMOVE)) {
			TranslateMessage(gameBase->GetMsg());
			DispatchMessage(gameBase->GetMsg());
		} else {

			gameBase->BeginFlame();


			gameBase->Update();
			
			

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

