#pragma once
#include "Transform.h"
#include "Vector3.h"
#include <vector>
#include "CSVManager.h"
#include <memory>
class Object3d;
class GameBase;
class Camera;

class MapchipField {

	static const int kWidth = 20;
	static const int kHeight = 20;
	static const float kTileSize;

	int field[kHeight][kWidth];

	// ★ タイル用の Object3d をまとめて持つ
	std::unique_ptr<Object3d> fieldObj = nullptr;

	Camera* camera_ = nullptr;
	Transform transform_;

public:
	MapchipField();
	~MapchipField();

	void Initialize(Camera* camera);
	void Update();
	void Draw();
	void SetCamera(Camera* camera) { camera_ = camera; }
	bool IsWall(int x, int y) const;
	void LoadFromCSV(const std::string& filename);
	static void WorldToTile(const Vector3& pos, int& tx, int& ty);
};
