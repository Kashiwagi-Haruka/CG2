#include"GameBase.h"
#include "DX12.h"
// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {

	GameBase* gameBase=new GameBase;
	DX12* dx12 = new DX12;
	//エンジンの初期化
	gameBase->Initialize(L"CG2", 1280, 720);
	dx12->Initialize();
	SetUnhandledExceptionFilter(dx12->ExportDump);

	

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
	
	return 0;

}