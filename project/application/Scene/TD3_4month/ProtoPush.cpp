#include "ProtoPush.h"

#include "Input.h"
#include "Object3d/Object3dCommon.h"
#include <numbers>
#ifdef USE_IMGUI
#include <imgui.h>
#endif

ProtoPush::ProtoPush() {
	camera_ = std::make_unique<Camera>();
	debugCamera_ = std::make_unique<DebugCamera>();
}

void ProtoPush::Initialize() {
	camera_->SetTransform({
	    {1.0f, 1.0f, 1.0f  },
        {0.1f, 0.0f, 0.0f  },
        {0.0f, 4.0f, -18.0f}
    });
	camera_->Update();

	debugCamera_->Initialize();
	debugCamera_->SetTranslation(camera_->GetTransform().translate);

	const std::array<Transform, 6> roomTransforms = {
	    Transform{{20.0f, 1.0f, 20.0f}, {std::numbers::pi_v<float> * 0.5f, 0.0f, 0.0f},  {0.0f, -1.0f, 0.0f} }, // 床
	    Transform{{20.0f, 1.0f, 20.0f}, {std::numbers::pi_v<float> * -0.5f, 0.0f, 0.0f}, {0.0f, 8.0f, 0.0f}  }, // 天井
	    Transform{{20.0f, 1.0f, 9.0f},  {0.0f, 0.0f, 0.0f},                              {0.0f, 3.5f, 10.0f} }, // 奥壁
	    Transform{{20.0f, 1.0f, 9.0f},  {0.0f, std::numbers::pi_v<float>, 0.0f},         {0.0f, 3.5f, -10.0f}}, // 手前壁
	    Transform{{20.0f, 1.0f, 9.0f},  {0.0f, std::numbers::pi_v<float> * 0.5f, 0.0f},  {-10.0f, 3.5f, 0.0f}}, // 左壁
	    Transform{{20.0f, 1.0f, 9.0f},  {0.0f, std::numbers::pi_v<float> * -0.5f, 0.0f}, {10.0f, 3.5f, 0.0f} }, // 右壁
	};

	for (size_t i = 0; i < roomBoards_.size(); ++i) {
		roomBoards_[i] = std::make_unique<Primitive>();
		roomBoards_[i]->Initialize(Primitive::Plane, "Resources/3d/debugBox.png");
		roomBoards_[i]->SetTransform(roomTransforms[i]);
		roomBoards_[i]->SetColor({0.85f, 0.85f, 0.85f, 1.0f});
		roomBoards_[i]->SetEnableLighting(true);
		roomBoards_[i]->SetCamera(camera_.get());
	}

	centerDivider_ = std::make_unique<Primitive>();
	centerDivider_->Initialize(Primitive::Plane, "Resources/3d/debugBox.png");
	centerDivider_->SetTransform({
	    {20.0f, 1.0f,	                         9.0f},
        {0.0f,  std::numbers::pi_v<float> * 0.5f, 0.0f},
        {0.0f,  3.5f,                             0.0f}
    });
	centerDivider_->SetColor({0.6f, 0.8f, 1.0f, 1.0f});
	centerDivider_->SetEnableLighting(true);
	centerDivider_->SetCamera(camera_.get());
}

void ProtoPush::Update() {
	UpdateCamera();

	for (auto& board : roomBoards_) {
		board->Update();
	}
	centerDivider_->Update();
}

void ProtoPush::Draw() {
	Object3dCommon::GetInstance()->DrawCommon();
	for (auto& board : roomBoards_) {
		board->Draw();
	}

	Object3dCommon::GetInstance()->DrawCommonNoCull();
	centerDivider_->Draw();
}

void ProtoPush::Finalize() {}

void ProtoPush::UpdateCamera() {
	if (Input::GetInstance()->TriggerKey(DIK_F1)) {
		useDebugCamera_ = !useDebugCamera_;
	}

	Camera* drawCamera = camera_.get();
	if (useDebugCamera_) {
		debugCamera_->Update();
		camera_->SetViewProjectionMatrix(debugCamera_->GetViewMatrix(), debugCamera_->GetProjectionMatrix());
	} else {
		camera_->Update();
	}

	Object3dCommon::GetInstance()->SetDefaultCamera(drawCamera);

#ifdef USE_IMGUI
	if (ImGui::Begin("ProtoPush")) {
		ImGui::Checkbox("Use Debug Camera (F1)", &useDebugCamera_);
		ImGui::Text("Primitive room scene");
		ImGui::End();
	}
#endif
}