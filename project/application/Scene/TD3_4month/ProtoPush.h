#pragma once
#include "BaseScene.h"
#include "Camera.h"
#include "DebugCamera.h"
#include "Primitive/Primitive.h"
#include <array>
#include <memory>

class ProtoPush : public BaseScene {
public:
	ProtoPush();
	~ProtoPush() override = default;

	void Initialize() override;
	void Update() override;
	void Draw() override;
	void Finalize() override;

private:
	std::unique_ptr<Camera> camera_ = nullptr;
	std::unique_ptr<DebugCamera> debugCamera_ = nullptr;
	bool useDebugCamera_ = false;

	std::array<std::unique_ptr<Primitive>, 6> roomBoards_{};
	std::unique_ptr<Primitive> centerDivider_ = nullptr;

	void UpdateCamera();
};