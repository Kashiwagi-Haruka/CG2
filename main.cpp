#include"GameBase.h"

// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {

	GameBase* gameBase=new GameBase;

	//エンジンの初期化
	gameBase->Initialize(L"CG2", 1280, 720);
	
	SetUnhandledExceptionFilter(gameBase->ExportDump);

	

	while (gameBase->IsMsgQuit())
	{
		if (PeekMessage(gameBase->GetMsg(), NULL, 0, 0, PM_REMOVE)) {
			TranslateMessage(gameBase->GetMsg());
			DispatchMessage(gameBase->GetMsg());
		} else {

			

			//ゲームの処理


		}



	}

	//出力ウィンドウへの文字出力
	gameBase->OutPutLog();
	gameBase->ResourceRelease();
	gameBase->CheackResourceLeaks();

	return 0;

}