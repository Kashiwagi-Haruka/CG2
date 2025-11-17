#pragma once
#include "Transform.h"
#include "Vector3.h"
#include <vector>
#include "CSVManager.h"

class Object3d;
class GameBase;
class Camera;

class MapchipField {

	static const int kWidth = 10;
	static const int kHeight = 10;
	static const float kTileSize;

	int field[kHeight][kWidth];

	// ★ タイル用の Object3d をまとめて持つ
	std::vector<Object3d*> tiles;

	Camera* camera_ = nullptr;

public:
	MapchipField();
	~MapchipField();

	void Initialize(GameBase* gameBase, Camera* camera);
	void Update();
	void Draw(GameBase* gameBase);

	bool IsWall(int x, int y) const;
	void LoadFromCSV(const std::string& filename);
	static void WorldToTile(const Vector3& pos, int& tx, int& ty);
};
