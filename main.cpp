#include"GameBase.h"

// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {

	CoInitializeEx(0, COINITBASE_MULTITHREADED);
	GameBase* gameBase=new GameBase;

	//エンジンの初期化
	gameBase->Initialize(L"CG2", 1280, 720);
	
	SetUnhandledExceptionFilter(gameBase->ExportDump);

	Vector3 position[3]{
		{-20,0,0,},
		{20,0,0},
		{0,20,0}
	};
	Vector2 texcoord[3] = {{0.0f, 1.0f}, {0.5f, 0.0f},{1.0f, 1.0f}};
	
	
	while (gameBase->IsMsgQuit())
	{
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
	gameBase->ResourceRelease();
	gameBase->CheackResourceLeaks();
	CoUninitialize();
	return 0;

}

