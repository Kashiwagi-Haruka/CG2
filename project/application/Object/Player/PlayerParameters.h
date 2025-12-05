#pragma once

struct Parameters{

	float accelationRate = 0.1f;       // 移動時の加速量（フレームごとに加速する値）
	float accelationMax = 0.25f;       // 横移動の最大速度
	float decelerationRate = 0.15f;    // キーを離した時の減速率
	float jumpPower = 0.70f;            // ジャンプの初速（上方向の初速）
	float jumpDuration = 0.5f;         // ジャンプを維持できる入力時間（未使用っぽいが意味はこれ）
	float jumpTimerMax = 0.01f;        // ジャンプ中状態を継続できる最大タイマー（非常に短く設定されている）
	float gravity = 0.98f / 10.0f/2.0f;     // 重力（毎フレーム下に加える値）
	float bulletRadius = 0.01f;        // 弾の当たり判定用の半径（小さい円）
	int hpMax_ = 10000;                // プレイヤーの最大HP
	float dashMagnification = 2.0f;    // ダッシュ時の速度倍率（通常速度に掛ける）
	float doubleTapThreshold_ = 30.0f; // ダブルタップと判定するためのフレーム間隔（30フレーム以内）

	int MaxLevel;
	int MaxEXP;
	int Level;
	int EXP;
};

