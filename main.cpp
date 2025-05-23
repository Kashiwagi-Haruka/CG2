#include"GameBase.h"
#include "GameScene.h"
#define _USE_MATH_DEFINES
#include <math.h>
#include <vector>
#include <random>
#include <array>
#include <cstdlib>  // srand, rand
#include <ctime>    // time
  	struct SpreadTri {
	float progress; // 0(奥)→1(手前)
	Vector2 dir;    // ばら撒く方向の単位ベクトル
};

static std::array<SpreadTri, 10> spreadTris;
// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {

	CoInitializeEx(0, COINITBASE_MULTITHREADED);
	GameBase* gameBase=new GameBase;
	
	//エンジンの初期化
	gameBase->Initialize(L"CG2", 1280, 720);
	
	SetUnhandledExceptionFilter(gameBase->ExportDump);

	 // 三角形の頂点・UV・カラーを用意

	
	// main.cpp の先頭あたり
	// #include～のすぐ下あたりに
	// 共通パラメータ（TriangleScene 等）
	static float commonTriColor[4] = {1.0f, 1.0f, 1.0f, 1.0f};
	static float commonTranslate[3] = {0.0f, 0.0f, 0.0f};
	static float commonRotateEuler[3] = {0.0f, 0.0f, 0.0f};
	static float commonScale[3] = {1.0f, 1.0f, 1.0f};

	// 深度バッファ用デモシーンパラメータ
	static float depthTriColorA[4] = {1.0f, 0.0f, 0.0f, 1.0f};
	static float depthTriColorB[4] = {0.0f, 1.0f, 0.0f, 1.0f};
	static float depthTranslateA[3] = {0.0f, 0.0f, 0.0f};
	static float depthTranslateB[3] = {0.0f, 0.0f, 0.1f}; // B を少し手前に
	static float depthRotateEulerA[3] = {0.0f, 56.0f, 0.0f};
	static float depthRotateEulerB[3] = {0.0f, 56.0f, 0.0f};
	static float depthScaleA[3] = {1.0f, 1.0f, 1.0f};
	static float depthScaleB[3] = {1.0f, 1.0f, 1.0f};
	// 三角形1
	Vector3 basePos1[3] = {
	    {-1.0f, 0.0f, 0.0f},
        {0.0f,  0.5f,  0.0f},
        {1.0f,  0.0f, 0.0f}
    };

	// 三角形2
	Vector3 basePos2[3] = {
	    {0.0f, 0.0f, -0.5f},
        {0.0f, 1.0f, 0.0f },
        {0.0f, 0.0f, 0.5f }
    };
	
	// 三角形の頂点初期位置
	static Vector3 basePos[3] = {
	    {-1.0f, -0.5f, 0.0f},
        {0.0f,  0.5f,  0.0f},
        {1.0f,  -0.5f, 0.0f}
    };
	// UV
	static Vector2 baseUV[3] = {
	    {0.0f, 1.0f},
        {0.5f, 0.0f},
        {1.0f, 1.0f}
    };
	static int sceneIndex = 0;  
	// テクスチャ選択

	
	const char* scenes[] = {"TriangleScene", "ParticleScene", "DepthBuffer"};

	Function function;
	

	int checkerId = gameBase->LoadTexture("Resources/uvChecker.png");

   
	struct Particle {
		Vector3 pos; // ワールド空間での座標
		Vector3 rot; // 回転角（ラジアン）
		float speed; // カメラに向かう速度
		float scale; // 大きさ
	};

	// パーティクル群を保持する静的変数
	static std::vector<Particle> particles;
	static bool init = false;

	if (!init) {
		std::mt19937 rng{std::random_device{}()};
		std::uniform_real_distribution<float> distAngle(0, 2 * float(M_PI));
		std::uniform_real_distribution<float> distRadius(5.0f, 20.0f);
		std::uniform_real_distribution<float> distDepth(50.0f, 100.0f);
		std::uniform_real_distribution<float> distSpeed(10.0f, 25.0f);
		std::uniform_real_distribution<float> distScale(0.05f, 0.2f);

		const int N = 200;
		particles.reserve(N);
		for (int i = 0; i < N; ++i) {
			float a = distAngle(rng);
			float r = distRadius(rng);
			particles.push_back({
			    {r * std::cos(a), r * std::sin(a), distDepth(rng)},
                {distAngle(rng),  distAngle(rng),  0.0f          },
                distSpeed(rng), distScale(rng)
            });
		}
		init = true;
	}

	static int texIdx = 0;
   const char* names[] = {
	   "Checker"};

   // ① 三角形毎のパラメータをまとめる構造体
   struct DepthTriParam {
	   Vector3 scale;       // {sx, sy, sz}
	   Vector3 rotateEuler; // {rx, ry, rz}（度数法）
	   Vector3 translate;   // {tx, ty, tz}
	   Vector4 color;       // RGBA
   };

   // ② １０個分のパラメータを配列で用意（値はサンプルです）
   static DepthTriParam tris[10] = {
	   {{1, 1, 1},          {0, 0, 0},   {0, 0, 10},   {1, 0, 0, 1}},
       {{0.5f, 0.5f, 0.5f}, {45, 45, 0}, {2, 1, 15},   {0, 1, 0, 1}},
       {{0.8f, 0.8f, 0.8f}, {0, 90, 0},  {-2, -1, 20}, {0, 0, 1, 1}},
	   // …以下 7 件分を同様に初期化…
   };

   // ③ ワールド→投影前のローカル三角形頂点（basePos1 と同じ形状）
   static Vector3 baseTri[3] = {
	   {-0.5f, 0.5f,  0.0f},
	   {0.5f,  0.5f,  0.0f},
	   {0.0f,  -0.5f, 0.0f},
   };

   // ④ UV は全頂点同じ（テクスチャ使わない場合はダミーでOK）
   static Vector2 uv[3] = {
	   {0, 0},
       {0, 0},
       {0, 0}
   };
  
 static const float startY = -0.9f;    // 列の一番上のYオフセット基準
   static const float stepY = 1.8f / 9;  // 三角を10個に並べる間隔
   static const float minGroupY = -1.2f; // 下端判定（画面外ぎりぎり）
   static const float maxGroupZ = 0.5f;  // 奥へ移動しきるZ
   static const float speedY = 0.004f;   // Y移動速度
   static const float speedZ = 0.004f;   // Z移動速度
   Vector4 PColor = {1.0f,1.0f,1.0f,1.0f};
   static float spinAngle = 0.0f; // フレーム間で角度を保持

   // main.cpp の先頭付近 or GameBase::Draw() の最初あたりで
   //  ────────────────
   // テクスチャの幅・高さが W, H
   const float W = static_cast<float>(512);
   const float H = static_cast<float>(512);

   // 白点を置いた座標が (W-1, H-1) のときの UV
   static const Vector2 whiteUV = {(W - 0.5f) / W, (H - 0.5f) / H};
   // ────────────────

	while (gameBase->IsMsgQuit())
	{
		if (PeekMessage(gameBase->GetMsg(), NULL, 0, 0, PM_REMOVE)) {
			TranslateMessage(gameBase->GetMsg());
			DispatchMessage(gameBase->GetMsg());
		} else {
			// フレーム開始
			gameBase->BeginFlame();

			// 描画コード例
			Vector4 color{commonTriColor[0], commonTriColor[1], commonTriColor[2], commonTriColor[3]};
			// ImGui UI 作成のみ
			// ImGui UI
			ImGui::Begin("Settings");
			ImGui::Combo("Scene", &sceneIndex, scenes, IM_ARRAYSIZE(scenes));
			

			if (sceneIndex == 0) {
				ImGui::Text("Transform");
				ImGui::DragFloat3("Translate", commonTranslate, 0.01f);
				ImGui::DragFloat3("Rotate (deg)", commonRotateEuler, 1.0f);
				ImGui::DragFloat3("Scale", commonScale, 0.01f);
				ImGui::Separator();
				ImGui::Text("Color");
				ImGui::ColorEdit4("Tri Color", commonTriColor);
			}
			if (sceneIndex == 2) {
				ImGui::Text("Triangle A");
				ImGui::DragFloat3("Translate A", depthTranslateA, 0.01f);
				ImGui::DragFloat3("Rotate A", depthRotateEulerA, 1.0f);
				ImGui::DragFloat3("Scale A", depthScaleA, 0.01f);
				
				ImGui::Separator();
				ImGui::Text("Triangle B");
				ImGui::DragFloat3("Translate B", depthTranslateB, 0.01f);
				ImGui::DragFloat3("Rotate B", depthRotateEulerB, 1.0f);
				ImGui::DragFloat3("Scale B", depthScaleB, 0.01f);
				ImGui::ColorEdit4("Color", depthTriColorB);
			}
			ImGui::End();
			;
				// 選択中のテクスチャを取得
			
	            Texture& cur = gameBase->GetTexture(texIdx);
	
			static bool spreadInit = false;
			    switch (sceneIndex) {
			    case 0: {
				    auto world = function.MakeAffineMatrix(
				        {commonScale[0], commonScale[1], commonScale[2]},
				        {commonRotateEuler[0] * (float)M_PI / 180.0f, commonRotateEuler[1] * (float)M_PI / 180.0f, commonRotateEuler[2] * (float)M_PI / 180.0f},
				        {commonTranslate[0], commonTranslate[1], commonTranslate[2]});
				    Vector3 pos[3];
				    for (int i = 0; i < 3; ++i)
					    pos[i] = function.Transform(basePos[i], world);
				    Vector4 col = {commonTriColor[0], commonTriColor[1], commonTriColor[2], commonTriColor[3]};
				    gameBase->DrawTriangle(pos, baseUV, col, cur);
			    } break;
			    case 1: {
				    // ─── 1) NDC そのまま通す ───
				    Matrix4x4 id{};
				    id.m[0][0] = id.m[1][1] = id.m[2][2] = id.m[3][3] = 1.0f;
				    gameBase->SetWorldViewProjection(id);

				    // ─── 2) 回転角度更新 ───
				    spinAngle += 0.02f; // 好みで回転スピード調整
				    float c = cosf(spinAngle), s = sinf(spinAngle);

				    // ─── 3) ランダム落下初期化 ───
				    static bool initialized = false;
				    struct Tri {
					    float x, y, z, speed;
				    };
				    static const int TRI_COUNT = 50;
				    static Tri tris[TRI_COUNT];
				    if (!initialized) {
					    initialized = true;
					    srand((unsigned)time(nullptr));
					    for (int i = 0; i < TRI_COUNT; ++i) {
						    tris[i].x = (rand() / float(RAND_MAX)) * 2.0f - 1.0f;
						    tris[i].y = (rand() / float(RAND_MAX)) * 2.0f + 1.0f;
						    float t = rand() / float(RAND_MAX);
						    tris[i].z = -1.0f + (t * t) * 2.0f; // t² によって手前（-1）寄りに偏る
						    tris[i].speed = 0.005f + (rand() / float(RAND_MAX)) * 0.015f;
					    }
				    }

				    // ─── 4) ローカル三角形（単位サイズ）＆色・UV ───
				    constexpr Vector3 localTri[3] = {
				        {-0.05f, 0.05f,  0.0f},
				        {0.05f,  0.05f,  0.0f},
				        {0.0f,   -0.05f, 0.0f},
				    };
				    Vector4 triColor = {0.5f, 0.8f, 0.8f, 0.5f};
				    Vector2 uv = {0.0f, 0.0f};

				    // ─── 5) 描画ループ ───
				    for (int i = 0; i < TRI_COUNT; ++i) {
					    // 位置更新
					    tris[i].y -= tris[i].speed;
					    if (tris[i].y < -1.2f) {
						    tris[i].x = (rand() / float(RAND_MAX)) * 2.0f - 1.0f;
						    tris[i].y = 1.2f + (rand() / float(RAND_MAX)) * 1.0f;
						    float t = rand() / float(RAND_MAX);
						    const float gamma = 0.7f;
						    float u = powf(t, gamma);
						    tris[i].z = -1.0f + u * 2.0f;
						    tris[i].speed = 0.005f + (rand() / float(RAND_MAX)) * 0.015f;
					    }

					    // 回転＋平行移動を手計算
					    Vector3 tri[3];
					    for (int v = 0; v < 3; ++v) {
						    float x0 = localTri[v].x;
						    float y0 = localTri[v].y;
						    // Z 軸回転
						    float xr = x0 * c - y0 * s;
						    float yr = x0 * s + y0 * c;
						    // NDC にマッピング
						    tri[v] = {xr + tris[i].x, yr + tris[i].y, tris[i].z};
					    }

					    gameBase->DrawTriangle(tri, &uv, triColor, cur);
				    }
				    break;
			    }

			case 2: {
				auto wA = function.MakeAffineMatrix(
				    {depthScaleA[0], depthScaleA[1], depthScaleA[2]},
				    {depthRotateEulerA[0] * (float)M_PI / 180.0f, depthRotateEulerA[1] * (float)M_PI / 180.0f, depthRotateEulerA[2] * (float)M_PI / 180.0f},
				    {depthTranslateA[0], depthTranslateA[1], depthTranslateA[2]});
				Vector3 pA[3];
				for (int i = 0; i < 3; ++i)
					pA[i] = function.Transform(basePos1[i], wA);
				Vector4 cA = {depthTriColorA[0], depthTriColorA[1], depthTriColorA[2], depthTriColorA[3]};
				gameBase->DrawTriangle(pA, baseUV, cA, cur);

				auto wB = function.MakeAffineMatrix(
				    {depthScaleB[0], depthScaleB[1], depthScaleB[2]},
				    {depthRotateEulerB[0] * (float)M_PI / 180.0f, depthRotateEulerB[1] * (float)M_PI / 180.0f, depthRotateEulerB[2] * (float)M_PI / 180.0f},
				    {depthTranslateB[0], depthTranslateB[1], depthTranslateB[2]});
				Vector3 pB[3];
				for (int i = 0; i < 3; ++i)
					pB[i] = function.Transform(basePos2[i], wB);
				Vector4 cB = {depthTriColorB[0], depthTriColorB[1], depthTriColorB[2], depthTriColorB[3]};
				gameBase->DrawTriangle(pB, baseUV, cB, cur);
				} break;
				}


       
	


		gameBase->EndFlame();
			//ゲームの処理


			}



		}

	//出力ウィンドウへの文字出力
	gameBase->OutPutLog();
	gameBase->ResourceRelease();
	gameBase->CheackResourceLeaks();
	CoUninitialize();
	return 0;

	}

