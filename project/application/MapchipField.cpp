#include "MapchipField.h"
#include "GameBase.h"
#include "ModelManeger.h"
#include "Object3d.h"

const float MapchipField::kTileSize = 1.0f;

MapchipField::MapchipField() {

	// 壁データ
	for (int y = 0; y < kHeight; y++) {
		for (int x = 0; x < kWidth; x++) {
			if (x == 0 || y == 0 || x == kWidth - 1 || y == kHeight - 1) {
				field[y][x] = 1;
			} else {
				field[y][x] = 0;
			}
		}
	}

	// モデル読み込み（map.obj）
	ModelManeger::GetInstance()->LoadModel("map");
}

MapchipField::~MapchipField() {
	for (auto t : tiles)
		delete t;
	tiles.clear();
}

void MapchipField::Initialize(GameBase* gameBase, Camera* camera) {

	camera_ = camera;

	// ★ 壁タイルだけ Object3d を生成
	for (int y = 0; y < kHeight; y++) {
		for (int x = 0; x < kWidth; x++) {

			if (field[y][x] == 1) {

				Object3d* obj = new Object3d();
				obj->Initialize(gameBase->GetObject3dCommon());
				obj->SetCamera(camera_);
				obj->SetModel("map");

				// ★ タイルの位置へ移動
				Transform t;
				t.scale = {1, 1, 1};
				t.rotate = {0, 0, 0};
				t.translate = {x * kTileSize, 0.0f, -y * kTileSize};

				obj->SetScale(t.scale);
				obj->SetRotate(t.rotate);
				obj->SetTranslate(t.translate);

				tiles.push_back(obj);
			}
		}
	}
}
void MapchipField::LoadFromCSV(const std::string& filename) {
	CSVManager::GetInstance()->LoadCSV(filename);
	const auto& csv = CSVManager::GetInstance()->GetData();

	for (int y = 0; y < kHeight; y++) {
		for (int x = 0; x < kWidth; x++) {
			if (y < csv.size() && x < csv[y].size()) {
				field[y][x] = csv[y][x];
			} else {
				field[y][x] = 0;
			}
		}
	}
}

void MapchipField::Update() {
	for (auto t : tiles) {
		t->Update();
	}
}

void MapchipField::Draw(GameBase* gameBase) {
	gameBase->ModelCommonSet();
	for (auto t : tiles) {
		t->Draw();
	}
}

bool MapchipField::IsWall(int x, int y) const {
	if (x < 0 || y < 0 || x >= kWidth || y >= kHeight)
		return true;
	return field[y][x] == 1;
}

void MapchipField::WorldToTile(const Vector3& pos, int& tx, int& ty) {
	tx = (int)floor(pos.x / kTileSize);
	ty = (int)floor(-pos.z / kTileSize);
}
