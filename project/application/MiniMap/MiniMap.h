#pragma once
#include "Vector2.h"
#include "Vector3.h"
#include "Vector4.h"

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

class Camera;
class Object3d;
class Sprite;

class MiniMap {
	struct Entry {
		Object3d* object = nullptr;
		Vector4 color = {1.0f, 1.0f, 1.0f, 1.0f};
	};

	struct MarkerDrawData {
		Vector3 position = {0.0f, 0.0f, 0.0f};
		Vector4 color = {1.0f, 1.0f, 1.0f, 1.0f};
	};

	static std::unique_ptr<MiniMap> instance_;

	std::unique_ptr<Camera> camera_;
	std::unique_ptr<Camera> miniMapCamera_;
	std::unique_ptr<Object3d> markerObject_;
	std::unique_ptr<Sprite> miniMapBackSprite_;
	std::unique_ptr<Sprite> miniMapFrameSprite_;
	std::unique_ptr<Sprite> markerSprite_;
	std::unordered_map<std::string, Entry> entries_;
	std::vector<MarkerDrawData> visibleMarkers_;
	Vector3 playerTranslate_ = {0.0f, 0.0f, 0.0f};
	float range_ = 25.0f;
	float markerHeight_ = 0.1f;
	Vector3 minimapAnchorOffset_ = {-14.0f, 30.0f, 22.0f};
	Vector2 miniMapScreenCenter_ = {130.0f, 590.0f};
	float miniMapRadius_ = 90.0f;
	uint32_t miniMapBackTextureHandle_ = 0;
	uint32_t miniMapFrameTextureHandle_ = 0;
	std::string stageName_;

	void UpdateCamera();
	void UpdateVisibleMarkers();

public:
	MiniMap() = default;
	~MiniMap() = default;
	MiniMap(const MiniMap&) = delete;
	MiniMap& operator=(const MiniMap&) = delete;

	static MiniMap* GetInstance();
	static void Finalize();

	void Initialize();
	void Update();
	void Draw();
	void AddObject(std::string name, Object3d* object, Vector4 color);
	void SetPlayerTranslate(Vector3 translate);
	void SetStage(std::string stageName);
	void Reset();
};