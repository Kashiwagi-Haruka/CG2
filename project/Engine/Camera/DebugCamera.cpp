#define NOMINMAX
#include "DebugCamera.h"
#include "GameBase.h"
#include "ImGuiManager.h"
#include "Input.h"
#include <algorithm>
void DebugCamera::Initialize() {
	// ViewMatrixの作成
	Matrix4x4 cameraMatrix = Function::MakeAffineMatrix(
	    {1, 1, 1},   // スケールは1
	    rotation_,   // 初期回転（デフォルト: {0, 0, 0}）
	    translation_ // 初期位置（デフォルト: {0, 0, -50}）
	);

	viewMatrix_ = Function::Inverse(cameraMatrix);

	// ProjectionMatrixの作成（FOV: 0.45f * π、アスペクト比: 16:9、近クリップ: 0.1f、遠クリップ: 1000.0f）
	float fovY = 0.45f * 3.14159265f; // ラジアン
	float aspectRatio = 16.0f / 9.0f;
	float nearClip = 0.1f;
	float farClip = 1000.0f;

	projectionMatrix_ = Function::MakePerspectiveFovMatrix(fovY, aspectRatio, nearClip, farClip);
	matRot_ = Function::MakeIdentity4x4();
}

void DebugCamera::Update() {
	// ── ImGui でパラメータをいじれるように ──
	// ImGui::Begin("DebugCamera");
	// ImGui::SliderFloat3("Offset", &translation_.x, -100.0f, 1000.0f);
	// ImGui::SliderFloat3("Pivot", &pivot_.x, 0.0f, 1000.0f);
	// ImGui::End();

	const float rotSpeed = 0.005f;
	const float zoomSpeed = 0.01f;
	const float panSpeed = 0.02f;
	float dPitch = 0.0f;
	float dYaw = 0.0f;

	GameBase* gameBase = GameBase::GetInstance();
	Vector2 mouseMove = gameBase->GetMouseMove();
	const bool isShift = gameBase->PushKey(DIK_LSHIFT) || gameBase->PushKey(DIK_RSHIFT);
	const bool isLeftDrag = gameBase->PushMouseButton(Input::MouseButton::kLeft);

	if (isLeftDrag && !isShift) {
		dYaw = mouseMove.x * rotSpeed;
		dPitch = mouseMove.y * rotSpeed;
	} else if (isLeftDrag && isShift) {
		Vector3 right = {matRot_.m[0][0], matRot_.m[1][0], matRot_.m[2][0]};
		Vector3 up = {matRot_.m[0][1], matRot_.m[1][1], matRot_.m[2][1]};
		pivot_ += right * (-mouseMove.x * panSpeed);
		pivot_ += up * (mouseMove.y * panSpeed);
	}

	const float wheelDelta = gameBase->GetMouseWheelDelta();
	if (wheelDelta != 0.0f) {
		translation_.z += wheelDelta * zoomSpeed;
		translation_.z = std::min(translation_.z, -1.0f);
		translation_.z = std::max(translation_.z, -500.0f);
	}

	// ── 累積回転行列に今回フレーム分の回転を乗算 ──
	Matrix4x4 matRotDelta = Function::MakeIdentity4x4();
	matRotDelta = Function::Multiply(matRotDelta, Function::MakeRotateXMatrix(dPitch));
	matRotDelta = Function::Multiply(matRotDelta, Function::MakeRotateYMatrix(dYaw));
	matRot_ = Function::Multiply(matRotDelta, matRot_); // ★資料「回転行列の累積」と同じ

	// ── scale は GUI 値が大きいほどズームインにしたいので逆数を使う ──
	Vector3 invScale = {scale_.x, scale_.y, scale_.z};

	// ── 行列合成：Pivot→累積回転→Scale→Offset（Pivot→(R→S→T)）─
	Matrix4x4 pivotMat = Function::MakeTranslateMatrix(pivot_);
	Matrix4x4 scaleMat = Function::MakeScaleMatrix(invScale);
	Matrix4x4 offsetMat = Function::MakeTranslateMatrix(translation_);

	// world→camera 行列
	//    ① pivot へ
	//    ② 回転（累積）
	//    ③ スケール（ズーム）
	//    ④ オフセット（距離）
	Matrix4x4 worldCam = Function::Multiply(Function::Multiply(Function::Multiply(pivotMat, matRot_), scaleMat), offsetMat);

	// ── ビュー行列は逆行列を取ってセット ──
	viewMatrix_ = Function::Inverse(worldCam);

	// ── ビュー×プロジェクション行列を更新 ──
	viewProjectionMatrix_ = Function::Multiply(viewMatrix_, projectionMatrix_);
}