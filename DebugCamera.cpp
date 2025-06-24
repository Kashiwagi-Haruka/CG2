#include "DebugCamera.h"
#include "DirectInput.h"
#include "imGuiM.h"
void DebugCamera::Initialize() {
	// ViewMatrixの作成
	Matrix4x4 cameraMatrix = function.MakeAffineMatrix(
	    {1, 1, 1},   // スケールは1
	    rotation_,   // 初期回転（デフォルト: {0, 0, 0}）
	    translation_ // 初期位置（デフォルト: {0, 0, -50}）
	);

	viewMatrix_ = function.Inverse(cameraMatrix);

	// ProjectionMatrixの作成（FOV: 0.45f * π、アスペクト比: 16:9、近クリップ: 0.1f、遠クリップ: 1000.0f）
	float fovY = 0.45f * 3.14159265f; // ラジアン
	float aspectRatio = 16.0f / 9.0f;
	float nearClip = 0.1f;
	float farClip = 1000.0f;

	projectionMatrix_ = function.MakePerspectiveFovMatrix(fovY, aspectRatio, nearClip, farClip);
}

void DebugCamera::Update(uint8_t* key, uint8_t* preKey) {


	ImGui::Begin("DebugCamera");
	ImGui::SliderFloat3("Pivot", &pivot_.x, -100.0f, 100.0f); // ← 追加
	ImGui::SliderFloat3("Scale", &scale_.x, 0.1f, 10.0f);  
	ImGui::SliderFloat3("Translation", &translation_.x, -100.0f, 100.0f);
	ImGui::SliderFloat3("Rotation", &rotation_.x, -3.14f, 3.14f);
	ImGui::End();

	// キーボード操作（WASD + 矢印キー）
	const float moveSpeed = 0.5f;
	const float rotateSpeed = 0.05f;

	if (KeyDown(DIK_W,key)) {
		translation_.z += moveSpeed;
	}
	if (KeyDown(DIK_S,key)) {
		translation_.z -= moveSpeed;
	}
	if (KeyDown(DIK_A,key)) {
		translation_.x -= moveSpeed;
	}
	if (KeyDown(DIK_D,key)) {
		translation_.x += moveSpeed;
	}
	if (KeyDown(DIK_Q,key)) {
		translation_.y -= moveSpeed;
	}
	if (KeyDown(DIK_E,key)) {
		translation_.y += moveSpeed;
	}

	if (KeyDown(DIK_UP,key)) {
		rotation_.x += rotateSpeed;
	}
	if (KeyDown(DIK_DOWN,key)) {
		rotation_.x -= rotateSpeed;
	}
	if (KeyDown(DIK_LEFT,key)) {
		rotation_.y += rotateSpeed;
	}
	if (KeyDown(DIK_RIGHT,key)) {
		rotation_.y -= rotateSpeed;
	}

// ビュー行列を更新
	// ビュー行列の更新（DebugCamera.cpp より）
	Matrix4x4 pivotMat = function.MakeTranslateMatrix(pivot_);   
	 Vector3 invScale=scale_;
	if (scale_.x > 0.0f || scale_.y > 0.0f || scale_.z > 0.0f) {
	
	 invScale = {1.0f / scale_.x, 1.0f / scale_.y, scale_.z  };
	}
	// Pivot へ移動
	Matrix4x4 camMat = function.MakeAffineMatrix(invScale, rotation_, translation_); // Scale→Rotate→Translate をまとめて
	Matrix4x4 worldCam = function.Multiply(pivotMat, camMat);                      // Pivot→(S→R→T)
	viewMatrix_ = function.Inverse(worldCam);                                      // ワールド→ビュー
	viewProjectionMatrix_ = function.Multiply(viewMatrix_, projectionMatrix_);     // ビュー×プロジェクション
  


	
}

