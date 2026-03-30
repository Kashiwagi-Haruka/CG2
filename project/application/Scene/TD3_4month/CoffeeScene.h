#pragma once
#include "BaseScene.h"
#include "Camera.h"
#include "DebugCamera.h"
#include "GameObject/Coffee/Coffees.h"
#include "Light/CommonLight/DirectionalCommonLight.h"
#include "Primitive/Primitive.h"
#include "Transform.h"
#include <array>
#include <cstddef>
#include <memory>

class CoffeeScene : public BaseScene {
private:
	static constexpr size_t kWallCount_ = 6;
	std::array<std::unique_ptr<Primitive>, kWallCount_> roomWalls_{};
	std::unique_ptr<Camera> camera_ = nullptr;
	std::unique_ptr<DebugCamera> debugCamera_ = nullptr;
	std::unique_ptr<Coffees> coffee_ = nullptr;
	Transform cameraTransform_{};
	DirectionalCommonLight directionalLight_{};
	bool useDebugCamera_ = false;
	bool useDirectionalShadow_ = true;

public:
	CoffeeScene();
	~CoffeeScene() override = default;
	void Initialize() override;
	void Update() override;
	void Draw() override;
	void Finalize() override;
};