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
	matRot_ = function.MakeIdentity();
}

void DebugCamera::Update(uint8_t* key, uint8_t* /*preKey*/) {
	// ── ImGui でパラメータをいじれるように ──
	ImGui::Begin("DebugCamera");
	ImGui::SliderFloat3("Pivot", &pivot_.x, -100.0f, 100.0f);
	ImGui::SliderFloat3("Offset", &translation_.x, -100.0f, 100.0f);
	ImGui::SliderFloat3("Zoom (Scale)", &scale_.x, 0.1f, 10.0f);
	ImGui::End();

	// ── キー操作でフレーム毎の回転デルタを得る ──
	const float rotSpeed = 0.02f; // rad/frame
	float dPitch = 0, dYaw = 0;
	if (key[DIK_UP] & 0x80)
		dPitch = -rotSpeed;
	if (key[DIK_DOWN] & 0x80)
		dPitch = rotSpeed;
	if (key[DIK_LEFT] & 0x80)
		dYaw = -rotSpeed;
	if (key[DIK_RIGHT] & 0x80)
		dYaw = rotSpeed;

	// ── 累積回転行列に今回フレーム分の回転を乗算 ──
	Matrix4x4 matRotDelta = function.MakeIdentity();
	matRotDelta = function.Multiply(matRotDelta,function.MakeRotateXMatrix(dPitch));
	matRotDelta = function.Multiply(matRotDelta,function.MakeRotateYMatrix(dYaw));
	matRot_ = function.Multiply(matRotDelta , matRot_); // ★資料「回転行列の累積」と同じ

	// ── scale は GUI 値が大きいほどズームインにしたいので逆数を使う ──
	Vector3 invScale = {1.0f / scale_.x, 1.0f / scale_.y, 1.0f / scale_.z};

	// ── 行列合成：Pivot→累積回転→Scale→Offset（Pivot→(R→S→T)）─
	Matrix4x4 pivotMat = function.MakeTranslateMatrix(pivot_);
	Matrix4x4 scaleMat = function.MakeScaleMatrix(invScale);
	Matrix4x4 offsetMat = function.MakeTranslateMatrix(translation_);

	// world→camera 行列
	//    ① pivot へ
	//    ② 回転（累積）
	//    ③ スケール（ズーム）
	//    ④ オフセット（距離）
	Matrix4x4 worldCam = function.Multiply(function.Multiply(function.Multiply(pivotMat , matRot_) , scaleMat) , offsetMat);

	// ── ビュー行列は逆行列を取ってセット ──
	viewMatrix_ = function.Inverse(worldCam);

	// ── ビュー×プロジェクション行列を更新 ──
	viewProjectionMatrix_ = function.Multiply(viewMatrix_, projectionMatrix_);
}