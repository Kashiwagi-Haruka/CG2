#include "EditorCamera.h"

#include "Camera.h"
#include "Function.h"
#include "Input.h"
#include "Object3d/Object3d.h"
#include "Object3d/Object3dCommon.h"


#include <algorithm>

#ifdef USE_IMGUI
#include "externals/imgui/imgui.h"
#endif

void EditorCamera::Reset() {
	cameraBillboardPrimitive_.reset();
	cameras_.clear();
	editorStatusMessage_.clear();
	DeactivatePreview();
}

void EditorCamera::DeactivatePreview() {
	isEditorPreviewCameraInitialized_ = false;
	wasEditorPreviewActiveLastFrame_ = false;
}

void EditorCamera::UpdateEditorPreview(bool isActive, const std::vector<Object3d*>& objects, const std::vector<Primitive*>& primitives) {
	if (!isActive) {
		wasEditorPreviewActiveLastFrame_ = false;
		return;
	}

	Object3dCommon* object3dCommon = Object3dCommon::GetInstance();
	if (!object3dCommon) {
		wasEditorPreviewActiveLastFrame_ = false;
		return;
	}

	Camera* sceneCamera = object3dCommon->GetDefaultCamera();
	if (!sceneCamera) {
		wasEditorPreviewActiveLastFrame_ = false;
		return;
	}

	if (!isEditorPreviewCameraInitialized_ || !wasEditorPreviewActiveLastFrame_) {
		editorPreviewCamera_.Initialize();
		editorPreviewCamera_.SetTransform(sceneCamera->GetTransform());
		isEditorPreviewCameraInitialized_ = true;
	}

	Input* input = Input::GetInstance();
	if (input) {
		input->SetIsCursorVisible(true);
		input->SetIsCursorStability(false);
	}

	editorPreviewCamera_.Update();
	sceneCamera->SetViewProjectionMatrix(editorPreviewCamera_.GetViewMatrix(), editorPreviewCamera_.GetProjectionMatrix());
	object3dCommon->SetDefaultCamera(sceneCamera);

	for (Object3d* object : objects) {
		if (object) {
			object->Update();
		}
	}

	for (Primitive* primitive : primitives) {
		if (primitive) {
			primitive->Update();
		}
	}

	wasEditorPreviewActiveLastFrame_ = true;
}

void EditorCamera::DrawCameraEditor() {
#ifdef USE_IMGUI
	Object3dCommon* object3dCommon = Object3dCommon::GetInstance();
	if (!object3dCommon) {
		ImGui::TextUnformatted("Object3dCommon unavailable");
		return;
	}

	Camera* camera = object3dCommon->GetDefaultCamera();
	if (!camera) {
		ImGui::TextUnformatted("No default camera");
		return;
	}

	bool cameraChanged = false;
	if (ImGui::TreeNode("Camera Transform")) {
		Vector3 scale = camera->GetScale();
		Vector3 rotate = camera->GetRotate();
		Vector3 translate = camera->GetTranslate();
		cameraChanged |= ImGui::DragFloat3("Scale", &scale.x, 0.01f);
		cameraChanged |= ImGui::DragFloat3("Rotate", &rotate.x, 0.01f);
		cameraChanged |= ImGui::DragFloat3("Translate", &translate.x, 0.01f);
		camera->SetScale(scale);
		camera->SetRotate(rotate);
		camera->SetTranslate(translate);
		ImGui::TreePop();
	}

	if (ImGui::TreeNode("Camera Projection")) {
		float fovY = camera->GetFovY();
		float aspectRatio = camera->GetAspectRatio();
		float nearZ = camera->GetNearZ();
		float farZ = camera->GetFarZ();
		cameraChanged |= ImGui::DragFloat("FovY", &fovY, 0.01f, 0.01f, 3.13f);
		cameraChanged |= ImGui::DragFloat("AspectRatio", &aspectRatio, 0.01f, 0.1f, 8.0f);
		cameraChanged |= ImGui::DragFloat("NearZ", &nearZ, 0.01f, 0.001f, 1000.0f);
		cameraChanged |= ImGui::DragFloat("FarZ", &farZ, 1.0f, 1.0f, 100000.0f);
		camera->SetFovY(fovY);
		camera->SetAspectRatio(aspectRatio);
		camera->SetNearClip(nearZ);
		camera->SetFarClip(farZ);
		ImGui::TreePop();
	}

	if (cameraChanged) {
		camera->Update();
	}

	if (ImGui::Button("Save Camera Editor")) {
		SaveEditorData(*camera);
	}
	ImGui::SameLine();
	if (ImGui::Button("Load Camera Editor")) {
		LoadEditorData(*camera);
	}
	if (!editorStatusMessage_.empty()) {
		ImGui::Text("%s", editorStatusMessage_.c_str());
	}
#endif
}

void EditorCamera::DrawCameraBillboards(bool isPlaying) {
#ifdef USE_IMGUI
	if (isPlaying || cameras_.empty()) {
		return;
	}

	Object3dCommon* object3dCommon = Object3dCommon::GetInstance();
	if (!object3dCommon) {
		return;
	}

	Camera* previewCamera = object3dCommon->GetDefaultCamera();
	if (!previewCamera) {
		return;
	}

	if (!cameraBillboardPrimitive_) {
		cameraBillboardPrimitive_ = std::make_unique<Primitive>();
		cameraBillboardPrimitive_->SetEditorRegistrationEnabled(false);
		cameraBillboardPrimitive_->Initialize(Primitive::Plane, "Resources/Editor/camera.png");
		cameraBillboardPrimitive_->SetEnableLighting(false);
	}

	cameraBillboardPrimitive_->SetCamera(previewCamera);
	cameraBillboardPrimitive_->SetColor({1.0f, 1.0f, 1.0f, 1.0f});
	cameraBillboardPrimitive_->SetShininess(1.0f);
	cameraBillboardPrimitive_->SetEnvironmentCoefficient(0.0f);

	Matrix4x4 billboardMatrix = Function::Inverse(previewCamera->GetViewMatrix());
	billboardMatrix.m[3][0] = 0.0f;
	billboardMatrix.m[3][1] = 0.0f;
	billboardMatrix.m[3][2] = 0.0f;

	object3dCommon->DrawCommonNoCull();
	for (Camera* camera : cameras_) {
		if (!camera || camera == previewCamera) {
			continue;
		}

		const Matrix4x4 worldMatrix = Function::Multiply(billboardMatrix, Function::MakeAffineMatrix(Vector3(1, 1, 1), Vector3(0, 0, 0), camera->GetTranslate()));
		cameraBillboardPrimitive_->SetWorldMatrix(worldMatrix);
		cameraBillboardPrimitive_->UpdateCameraMatrices();
		cameraBillboardPrimitive_->Draw();
	}
#endif
}

void EditorCamera::RegisterCamera(Camera* camera) {
	if (!camera) {
		return;
	}
	if (std::find(cameras_.begin(), cameras_.end(), camera) != cameras_.end()) {
		return;
	}
	cameras_.push_back(camera);
}

void EditorCamera::UnregisterCamera(Camera* camera) {
	if (!camera) {
		return;
	}
	auto it = std::remove(cameras_.begin(), cameras_.end(), camera);
	if (it != cameras_.end()) {
		cameras_.erase(it, cameras_.end());
	}
}

void EditorCamera::LoadEditorData(Camera& camera) { editorStatusMessage_ = camera.LoadEditorData() ? "Loaded camera editor settings" : "Load failed: Resources/JSON/cameraEditor.json"; }

void EditorCamera::SaveEditorData(const Camera& camera) {
	editorStatusMessage_ = camera.SaveEditorData() ? "Saved: Resources/JSON/cameraEditor.json" : "Save failed: Resources/JSON/cameraEditor.json";
}