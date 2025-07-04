#define NOMINMAX
#include "WaterController.h"
#include <cmath>
#include <string>
#include "Vector2.h"
#include "Vector3.h"
#include "imGuiM.h"

// スクリーン投影
Vector2 WaterController::ProjectToScreen(const Vector3& w, const Vector3& cam, const Vector3& tgt) {
	// カメラ座標変換
	Vector3 fwd = {tgt.x - cam.x, tgt.y - cam.y, tgt.z - cam.z};
	float fwdLen = sqrtf(fwd.x * fwd.x + fwd.y * fwd.y + fwd.z * fwd.z);
	fwd.x /= fwdLen;
	fwd.y /= fwdLen;
	fwd.z /= fwdLen;
	Vector3 up = {0, 1, 0};
	Vector3 right = {fwd.y * up.z - fwd.z * up.y, fwd.z * up.x - fwd.x * up.z, fwd.x * up.y - fwd.y * up.x};
	float rightLen = sqrtf(right.x * right.x + right.y * right.y + right.z * right.z);
	right.x /= rightLen;
	right.y /= rightLen;
	right.z /= rightLen;
	up = {right.y * fwd.z - right.z * fwd.y, right.z * fwd.x - right.x * fwd.z, right.x * fwd.y - right.y * fwd.x};
	Vector3 rel = {w.x - cam.x, w.y - cam.y, w.z - cam.z};
	float vx = rel.x * right.x + rel.y * right.y + rel.z * right.z;
	float vy = rel.x * up.x + rel.y * up.y + rel.z * up.z;
	float vz = rel.x * fwd.x + rel.y * fwd.y + rel.z * fwd.z;

	// パース投影
	float focal = 300.0f;
	float sx = vx * (focal / vz) + 1280 / 2;
	float sy = -vy * (focal / vz) + 720 / 2;
	return {sx, sy};

}

void WaterController::Initialize() {
	
	isCharging_ = false;
	isFired_ = false;

	camDistance_ = 800.0f;
	camAzimuth_ = 0.0f;
	camElevation_ = 0.2f;
	camTarget_ = {0.0f, 0.0f, 0.0f};
	// 格子初期化
	gridPoints3D_.resize(kRowCount + 1);
	for (int i = 0; i <= kRowCount; ++i) {
		gridPoints3D_[i].resize(kColCount + 1);
	}
}

void WaterController::StartCharge(const Vector2& mousePos, bool justPressed) {
	if (justPressed) {
	}
	mousePos_ = mousePos;
	isCharging_ = true;
	isFired_ = false;
	// マウスで環の中心を動かす
	camTarget_.x = mousePos.x - 640.0f;
	camTarget_.z = mousePos.y - 360.0f;
}

void WaterController::Update() {}

void WaterController::Fire() {
	isFired_ = true;
	isCharging_ = false;
}
// Catmull-Rom補間（0<=t<=1、4点 p0-p1-p2-p3 からp1-p2間をなめらかに補間）
Vector3 CatmullRom(const Vector3& p0, const Vector3& p1, const Vector3& p2, const Vector3& p3, float t) {
	float t2 = t * t;
	float t3 = t2 * t;
	return {
	    0.5f * ((2.0f * p1.x) + (-p0.x + p2.x) * t + (2.0f * p0.x - 5.0f * p1.x + 4.0f * p2.x - p3.x) * t2 + (-p0.x + 3.0f * p1.x - 3.0f * p2.x + p3.x) * t3),
	    0.5f * ((2.0f * p1.y) + (-p0.y + p2.y) * t + (2.0f * p0.y - 5.0f * p1.y + 4.0f * p2.y - p3.y) * t2 + (-p0.y + 3.0f * p1.y - 3.0f * p2.y + p3.y) * t3),
	    0.5f * ((2.0f * p1.z) + (-p0.z + p2.z) * t + (2.0f * p0.z - 5.0f * p1.z + 4.0f * p2.z - p3.z) * t2 + (-p0.z + 3.0f * p1.z - 3.0f * p2.z + p3.z) * t3)};
}
void WaterController::Draw(GameBase& gameBase) {
	// --- ImGuiカメラ操作 ---
	ImGui::Begin("Camera Orbit");
	ImGui::SliderFloat("Distance", &camDistance_, 100.0f, 3000.0f);
	ImGui::SliderAngle("Azimuth", &camAzimuth_, 0.0f, 360.0f);
	ImGui::SliderAngle("Elevation", &camElevation_, -89.0f, 89.0f);
	ImGui::SliderFloat3("Target", &camTarget_.x, -2000.0f, 2000.0f);
	ImGui::End();

	// 軌道カメラ計算
	camPos_.x = camTarget_.x + camDistance_ * cosf(camElevation_) * cosf(camAzimuth_);
	camPos_.y = camTarget_.y + camDistance_ * sinf(camElevation_);
	camPos_.z = camTarget_.z + camDistance_ * cosf(camElevation_) * sinf(camAzimuth_);

	float time = static_cast<float>(clock()) / CLOCKS_PER_SEC;

	int nodeCount = 9;
	int sectionCount = 180; // ←多いほどなめらか＆隙間なし
	float ribbonWidth = 40.0f;
	float radius = 250.0f;

// --- 螺旋状水流ノード列 ---
	std::vector<Vector3> nodes;
	float spiralTurns = 2.2f; // 螺旋の回転数
	float spiralTight = 1.0f; // 渦の締まり具合
	for (int n = 0; n < nodeCount; ++n) {
		float t = float(n) / (nodeCount - 1); // 0～1
		float angle = t * spiralTurns * 2.0f * 3.1415f + time * 0.5f;
		float spiralRadius = radius * (1.0f - t * 0.30f); // 外ほど太い→細く
		float z = -t * 350.0f + sinf(time * 0.7f + t * 7.0f) * 30.0f;
		nodes.push_back({cosf(angle) * spiralRadius, sinf(angle) * spiralRadius, z});
	}


	// スプラインで中心列
	std::vector<Vector3> centerList;
	for (int i = 0; i <= sectionCount; ++i) {
		float t = float(i) / sectionCount;
		float tNode = t * (nodeCount - 1);
		int idx = int(tNode);
		float localT = tNode - idx;
		int idx0 = std::max(0, idx - 1);
		int idx1 = idx;
		int idx2 = std::min(nodeCount - 1, idx + 1);
		int idx3 = std::min(nodeCount - 1, idx + 2);
		centerList.push_back(CatmullRom(nodes[idx0], nodes[idx1], nodes[idx2], nodes[idx3], localT));
	}

	std::vector<Vector3> leftList, rightList;
	for (int i = 0; i <= sectionCount; ++i) {
		// 進行方向
		Vector3 tangent;
		if (i < sectionCount) {
			Vector3& p0 = centerList[i];
			Vector3& p1 = centerList[i + 1];
			tangent = {p1.x - p0.x, p1.y - p0.y, p1.z - p0.z};
		} else {
			Vector3& p0 = centerList[i - 1];
			Vector3& p1 = centerList[i];
			tangent = {p1.x - p0.x, p1.y - p0.y, p1.z - p0.z};
		}
		float len = std::sqrt(tangent.x * tangent.x + tangent.y * tangent.y + tangent.z * tangent.z);
		tangent.x /= len;
		tangent.y /= len;
		tangent.z /= len;

		// XY平面直交法線
		Vector3 normal = {-tangent.y, tangent.x, 0.0f};
		float nlen = std::sqrt(normal.x * normal.x + normal.y * normal.y);
		normal.x /= nlen;
		normal.y /= nlen;

		// ★ここで水らしい“凹凸”を与える★
		// sectionCount・i・timeを使い、sin/cos/ノイズ混ぜてOK！
		float tWave = float(i) / sectionCount;
		float wave = sinf(i * 0.3f + time * 2.0f) * 1.95f    // ゆるい波
		             + sinf(i * 1.4f - time * 3.0f) * 0.9f  // 細かい波
		             + cosf(i * 0.85f + time * 1.7f) * 0.5f // さらにノイズ
		             + (rand() % 100 - 50) * 0.001f;         // 軽いランダム

		// 左右で異なるバリエーションを付けるとより水感アップ
		float leftWidth = ribbonWidth * (1.00f + 0.19f * wave + 0.05f * sinf(i + time * 1.1f));
		float rightWidth = ribbonWidth * (1.00f - 0.19f * wave + 0.05f * cosf(i - time * 0.6f));

		leftList.push_back({centerList[i].x + normal.x * leftWidth, centerList[i].y + normal.y * leftWidth, centerList[i].z});
		rightList.push_back({centerList[i].x - normal.x * rightWidth, centerList[i].y - normal.y * rightWidth, centerList[i].z});
	}

	// --- 端点列から必ず隣同士の点でQuadを構成 ---
	for (int i = 0; i < sectionCount; ++i) {
		// スクリーン座標
		Vector2 s_left0 = ProjectToScreen(leftList[i], camPos_, camTarget_);
		Vector2 s_right0 = ProjectToScreen(rightList[i], camPos_, camTarget_);
		Vector2 s_left1 = ProjectToScreen(leftList[i + 1], camPos_, camTarget_);
		Vector2 s_right1 = ProjectToScreen(rightList[i + 1], camPos_, camTarget_);

		Vector3 quadPos[4] = {
		    {s_left0.x,  s_left0.y,  0},
            {s_right0.x, s_right0.y, 0},
            {s_right1.x, s_right1.y, 0},
            {s_left1.x,  s_left1.y,  0}
        };

		// UV
		float t0 = float(i) / sectionCount;
		float t1 = float(i + 1) / sectionCount;
		float uvScroll = std::fmod(time * 0.13f, 1.0f);
		Vector2 quadUV[4] = {
		    {0, t0 + uvScroll},
            {1, t0 + uvScroll},
            {1, t1 + uvScroll},
            {0, t1 + uvScroll}
        };

		uint32_t color = 0x99C0FFFF;

		gameBase.DrawSpriteSheet(quadPos, quadUV, color);
	}










	// // 3Dしぶきパーティクル生成
	//if (splashes.size() < 80) {
	//	for (int tryCount = 0; tryCount < 2; ++tryCount) {
	//		int i = mt() % (kRowCount + 1);
	//		int j = mt() % (kColCount + 1);
	//		Vector3 base = gridPoints3D_[i][j];
	//		float angH = (mt() % 360) * 3.14159f / 180.0f;
	//		float angV = (mt() % 180) * 3.14159f / 180.0f - 3.14159f / 2.0f;
	//		float speed = 3.0f + (mt() % 30) / 10.0f;

	//		SplashParticle sp;
	//		sp.pos3 = base;
	//		sp.vel3 = {cosf(angH) * cosf(angV) * speed, sinf(angV) * speed - 2.0f, sinf(angH) * cosf(angV) * speed};
	//		sp.lifetime = 0.4f + (mt() % 40) / 100.0f;
	//		sp.age = 0.0f;
	//		// ← ここを大きめの値に変更
	//		sp.scale = 0.1f + (mt() % 50) / 400.0f;
	//		sp.angle = angH;
	//		splashes.push_back(sp);
	//	}
	//}

	//// 3Dしぶきパーティクル描画
	//for (auto it = splashes.begin(); it != splashes.end();) {
	//	// --- 物理更新 ---
	//	it->age += 1.0f / 60.0f;
	//	it->pos3.x += it->vel3.x;
	//	it->pos3.y += it->vel3.y;
	//	it->pos3.z += it->vel3.z;
	//	it->vel3.y += 0.4f; // 重力

	//	// α計算
	//	float alpha = 1.0f - (it->age / it->lifetime);
	//	if (alpha < 0.0f)
	//		alpha = 0.0f;
	//	uint32_t color = ((int)(220 * alpha) << 24) | 0x00FFFFFF;

	//	// スクリーン投影
	//	Vector2 splash2D = ProjectToScreen(it->pos3, camPos_, camTarget_);
	//	Vector2 spritePos{splash2D.x, splash2D.y};

	//	// --- スプライトシートの UV 計算 (3x3 枚と仮定) ---
	//	const int sheetCols = 3, sheetRows = 3;
	//	const float du = 1.0f / sheetCols;
	//	const float dv = 1.0f / sheetRows;
	//	// 表示したいセル番号 (0～8)。ランダムにするなら mt() % (sheetCols*sheetRows)
	//	int cellIndex = 0;
	//	int uIndex = cellIndex % sheetCols;
	//	int vIndex = cellIndex / sheetCols;
	//	float u0 = uIndex * du;
	//	float v0 = vIndex * dv;
	//	Vector2 uvQuad[4] = {
	//	    {u0,      v0     },
	//	    {u0 + du, v0     },
	//	    {u0,      v0 + dv},
	//	    {u0 + du, v0 + dv},
	//	};

	//	// --- 頂点クアッドの計算 ---
	//	float halfW = pieceW_ * it->scale * 0.5f;
	//	float halfH = pieceH_ * it->scale * 0.5f;
	//	Vector3 posQuad[4] = {
	//	    {spritePos.x - halfW, spritePos.y - halfH, 0.0f},
	//	    {spritePos.x + halfW, spritePos.y - halfH, 0.0f},
	//	    {spritePos.x + halfW, spritePos.y + halfH, 0.0f},
	//	    {spritePos.x - halfW, spritePos.y + halfH, 0.0f},
	//	};

	//	// --- 描画 ---
	//	gameBase.DrawSpriteSheet(posQuad, uvQuad, color);

	//	// --- 寿命終了チェック ---
	//	if (it->age > it->lifetime) {
	//		it = splashes.erase(it);
	//	} else {
	//		++it;
	//	}
	//
	//}
}
