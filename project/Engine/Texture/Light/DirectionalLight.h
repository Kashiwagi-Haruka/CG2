#pragma once
#include "Vector4.h"
#include "Vector3.h"
#include <wrl.h>
#include <d3d12.h>

struct DirectionalLight {
	Vector4 color;
	Vector3 direction;
	float intensity;
};

class Object3dCommon;
class DirectionalLightHelper {


	DirectionalLight* directionalLightData_ = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> directionalLightResource_ = nullptr;
	Object3dCommon* obj3dCommon_ = nullptr;

	public:

	void Initialize();
	void Update();
	



};
