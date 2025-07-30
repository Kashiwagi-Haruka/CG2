#include"GameBase.h"
#include "ResourceObject.h"
#include "DebugCamera.h"
#include "WaterController.h"
#include "ObjDraw.h"
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

	SoundData soundData1 = gameBase->SoundLoadWave("Resources/Alarm01.wav");

	gameBase->SoundPlayWave(soundData1);

	DebugCamera camera;
	camera.Initialize();
	Function function;
#pragma endregion

	
	enum class SceneName{

		objSprite,//スプライトと平面オブジェクト描画
		Sphere,//球の描画
		LambertianReflectance,//ライト
		HarfLambert,//ハーフライト
		UVtransform,//UVトランスフォーム
		Models,//複数モデル描画
		UtahTeapot,//Teapot.objの描画
		StanfordBunny,//bunny.objの描画
		MultiMesh,//multiMesh.objの描画
		MultiMaterial,//multiMaterial.objの描画
		MetaBall,//メタボールの描画
	};

	SceneName scene=SceneName::objSprite;

	WaterController water;
	water.Initialize();
	ObjDraw objDraw;
	objDraw.Initialize(*gameBase);
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

			ImGui::Begin("SceneChange");

			if (ImGui::Button("objSprite")) {
				scene = SceneName::objSprite;
			}
			if (ImGui::Button("Sphere")) {
				scene = SceneName::Sphere;
			}
			if (ImGui::Button("LambertianReflectance")) {
				scene = SceneName::LambertianReflectance;
			}
			if (ImGui::Button("HarfLambert")) {
				scene = SceneName::HarfLambert;
			}



			if (ImGui::Button("MetaBall")) {
				scene = SceneName::MetaBall;
			}
			ImGui::End();


			if (scene == SceneName::MetaBall) {
				gameBase->SetIsMetaBall(true);
			} else {
				gameBase->SetIsMetaBall(false);
				objDraw.HarfLightControl(*gameBase);
			}

			

			switch (scene) {

			case SceneName::objSprite:

				objDraw.DrawObjSprite(*gameBase, wvpMatrix);

				break;
			case SceneName::Sphere:


				objDraw.DrawSphere(*gameBase, wvpMatrix);


				break;
			case SceneName::LambertianReflectance:
				break;
			case SceneName::HarfLambert:
				break;
			case SceneName::UVtransform:
				break;
			case SceneName::Models:
				break;
			case SceneName::UtahTeapot:
				break;
			case SceneName::StanfordBunny:
				break;
			case SceneName::MultiMesh:
				break;
			case SceneName::MultiMaterial:
				break;
			case SceneName::MetaBall:

				
				water.Update();

				water.Draw(*gameBase, wvpMatrix);


				break;
			default:
				break;
			}
		
			
			

	

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

