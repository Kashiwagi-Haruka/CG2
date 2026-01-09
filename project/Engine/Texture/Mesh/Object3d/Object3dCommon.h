#pragma once
#include "Light/DirectionalLight.h"
#include "Light/PointLight.h"
#include "Light/SpotLight.h"
#include "PSO/CreatePSO.h"
#include <Windows.h>
#include <cstdint>
#include <memory>
#include <wrl.h>
class Camera;
class DirectXCommon;

class Object3dCommon {

private:
	static std::unique_ptr<Object3dCommon> instance;

	Camera* defaultCamera = nullptr;

	DirectXCommon* dxCommon_ = nullptr;

	HRESULT hr_;

	BlendMode blendMode_ = BlendMode::kBlendModeNone;
	BlendModeManager blendModeManeger_;

	std::unique_ptr<CreatePSO> pso_;

	// Directional Light（共通）
	DirectionalLight* directionalLightData_ = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> directionalLightResource_ = nullptr;
	PointLightSet* pointlightData_;
	Microsoft::WRL::ComPtr<ID3D12Resource> pointLightResource_ = nullptr;
	SpotLight* spotlightData_;
	Microsoft::WRL::ComPtr<ID3D12Resource> spotLightResource_ = nullptr;

public:
	static Object3dCommon* GetInstance();
	Object3dCommon();
	~Object3dCommon();
	void Initialize(DirectXCommon* dxCommon);
	void SetDefaultCamera(Camera* camera) { this->defaultCamera = camera; }
	Camera* GetDefaultCamera() const { return defaultCamera; };
	void DrawCommon();
	Microsoft::WRL::ComPtr<ID3D12Resource> CreateBufferResource(size_t sizeInBytes);
	DirectXCommon* GetDxCommon() const { return dxCommon_; };
	void SetBlendMode(BlendMode blendmode);
	BlendMode GetBlendMode() const { return blendMode_; }
	ID3D12Resource* GetDirectionalLightResource() const { return directionalLightResource_.Get(); }
	ID3D12Resource* GetPointLightResource() const { return pointLightResource_.Get(); }
	ID3D12Resource* GetSpotLightResource() const { return spotLightResource_.Get(); }
	void SetDirectionalLight(DirectionalLight& light);
	void SetPointLights(const PointLight* pointLights, uint32_t count);
	void SetSpotLight(SpotLight spotlight);
};