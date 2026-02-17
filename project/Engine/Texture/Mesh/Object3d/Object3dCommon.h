#pragma once
#include "Light/AreaLight.h"
#include "Light/DirectionalLight.h"
#include "Light/PointLight.h"
#include "Light/SpotLight.h"
#include "PSO/CreatePSO.h"
#include "Matrix4x4.h"
#include <Windows.h>
#include <cstdint>
#include <memory>
#include <string>
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
	BlendModeManager blendModeManager_;

	std::unique_ptr<CreatePSO> pso_;
	std::unique_ptr<CreatePSO> psoToon_;
	std::unique_ptr<CreatePSO> psoEmissive_;
	std::unique_ptr<CreatePSO> psoNoCull_;
	std::unique_ptr<CreatePSO> psoNoDepth_;
	std::unique_ptr<CreatePSO> psoNoCullDepth_;
	std::unique_ptr<CreatePSO> psoWireframe_;
	std::unique_ptr<CreatePSO> psoWireframeNoDepth_;
	std::unique_ptr<CreatePSO> psoLine_;
	std::unique_ptr<CreatePSO> psoLineNoDepth_;
	std::unique_ptr<CreatePSO> psoSkinning_;
	std::unique_ptr<CreatePSO> psoSkinningToon_;
	std::unique_ptr<CreatePSO> psoMirror_;
	std::unique_ptr<CreatePSO> psoShadow_;

	// Directional Light（共通）
	DirectionalLight* directionalLightData_ = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> directionalLightResource_ = nullptr;
	PointLight* pointlightData_ = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> pointLightResource_ = nullptr;
	PointLightCount* pointLightCountData_ = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> pointLightCountResource_ = nullptr;
	uint32_t pointLightSrvIndex_ = 0;
	SpotLight* spotLightData_ = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> spotLightResource_ = nullptr;
	SpotLightCount* spotLightCountData_ = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> spotLightCountResource_ = nullptr;
	uint32_t spotLightSrvIndex_ = 0;
	AreaLight* areaLightData_ = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> areaLightResource_ = nullptr;
	AreaLightCount* areaLightCountData_ = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> areaLightCountResource_ = nullptr;
	uint32_t areaLightSrvIndex_ = 0;
	uint32_t environmentMapSrvIndex_ = 0;
	uint32_t shadowMapSrvIndex_ = 0;
	std::string environmentMapPath_;
	Microsoft::WRL::ComPtr<ID3D12Resource> shadowMapResource_ = nullptr;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> shadowDsvHeap_ = nullptr;
	D3D12_VIEWPORT shadowViewport_{};
	D3D12_RECT shadowScissorRect_{};
	static constexpr uint32_t kShadowMapSize_ = 2048;
	bool isShadowMapPassActive_ = false;
	Vector3 shadowLightPosition_ = {0.0f, 80.0f, 0.0f};
	float shadowCameraNear_ = 0.1f;
	float shadowCameraFar_ = 300.0f;
	float shadowOrthoHalfWidth_ = 80.0f;
	float shadowOrthoHalfHeight_ = 80.0f;
	bool fullScreenGrayscaleEnabled_ = false;
	void DrawSet();

public:
	static Object3dCommon* GetInstance();
	Object3dCommon();
	~Object3dCommon();
	void Initialize(DirectXCommon* dxCommon);
	void Finalize();
	void SetDefaultCamera(Camera* camera) { this->defaultCamera = camera; }
	Camera* GetDefaultCamera() const { return defaultCamera; };
	void DrawCommon();
	void DrawCommonToon();
	void DrawCommonEmissive();
	void DrawCommonNoCull();
	void DrawCommonNoDepth();
	void DrawCommonNoCullDepth();
	void DrawCommonWireframeNoDepth();
	void DrawCommonLineNoDepth();
	void DrawCommonSkinning();
	void DrawCommonSkinningToon();
	void DrawCommonMirror();
	void DrawCommonShadow();
	void BeginShadowMapPass();
	void EndShadowMapPass();
	Microsoft::WRL::ComPtr<ID3D12Resource> CreateBufferResource(size_t sizeInBytes);
	DirectXCommon* GetDxCommon() const { return dxCommon_; };
	void SetBlendMode(BlendMode blendmode);
	BlendMode GetBlendMode() const { return blendMode_; }
	ID3D12Resource* GetDirectionalLightResource() const { return directionalLightResource_.Get(); }
	ID3D12Resource* GetPointLightCountResource() const { return pointLightCountResource_.Get(); }
	uint32_t GetPointLightSrvIndex() const { return pointLightSrvIndex_; }
	ID3D12Resource* GetSpotLightCountResource() const { return spotLightCountResource_.Get(); }
	uint32_t GetSpotLightSrvIndex() const { return spotLightSrvIndex_; }
	ID3D12Resource* GetAreaLightCountResource() const { return areaLightCountResource_.Get(); }
	uint32_t GetAreaLightSrvIndex() const { return areaLightSrvIndex_; }
	uint32_t GetEnvironmentMapSrvIndex() const { return environmentMapSrvIndex_; }
	uint32_t GetShadowMapSrvIndex() const { return shadowMapSrvIndex_; }
	bool IsShadowMapPassActive() const { return isShadowMapPassActive_; }
	void SetEnvironmentMapTexture(const std::string& filePath);
	void SetEnvironmentMapTextureResource(ID3D12Resource* resource, DXGI_FORMAT format);
	void SetDirectionalLight(DirectionalLight& light);
	void SetPointLights(const PointLight* pointLights, uint32_t count);
	void SetSpotLights(const SpotLight* spotLights, uint32_t count);
	void SetAreaLights(const AreaLight* areaLights, uint32_t count);
	Matrix4x4 GetDirectionalLightViewProjectionMatrix() const;
	void SetFullScreenGrayscaleEnabled(bool enable) { fullScreenGrayscaleEnabled_ = enable; }
	bool IsFullScreenGrayscaleEnabled() const { return fullScreenGrayscaleEnabled_; }
};