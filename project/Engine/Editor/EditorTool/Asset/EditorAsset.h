#pragma once
class EditorAsset {

public:
	enum class AssetCategory { None, Audio, Camera, Sprite, Primitive, Object3d, Material };

	void AudioRegister();
	void CameraRegister();
	void SpriteRegister();
	void PrimitiveRegister();
	void Object3dRegister();
	void MaterialRegister();

	void EditorDraw();

private:
	AssetCategory selectedCategory_ = AssetCategory::None;
};