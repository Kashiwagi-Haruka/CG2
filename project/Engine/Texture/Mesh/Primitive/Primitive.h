#pragma once
#include <cstdint>
#include "Matrix4x4.h"
#include "Transform.h"
#include "CameraForGPU.h"
#include <wrl.h>
#include <d3d12.h>

class Camera;

class Primitive {

	public:
	enum PrimitiveName {
		Plane,
		Circle,
		Ring,
		Sphere,
		Torus,
		Cylinder,
		Cone,
		Triangle,
		Box,
	};

	private:

		PrimitiveName primitiveName_;
	    struct alignas(256) TransformationMatrix {
		    Matrix4x4 WVP;                   // 64 バイト
		    Matrix4x4 World;                 // 64 バイト
		    Matrix4x4 WorldInverseTranspose; // ここで自動的に 128 バイト分のパディングが入って、
		                                     // sizeof(TransformationMatrix) == 256 になる
	    };

	    Transform transform_ = {
	        {1.0f, 1.0f, 1.0f},
	        {0.0f, 0.0f, 0.0f},
	        {0.0f, 0.0f, 0.0f},
	    };

	    Camera* camera_;

	    TransformationMatrix* transformationMatrixData_;
	    Microsoft::WRL::ComPtr<ID3D12Resource> transformResource_;
	    CameraForGpu* cameraData_;
	    Microsoft::WRL::ComPtr<ID3D12Resource> cameraResource_;
	    Matrix4x4 worldMatrix;
	    Matrix4x4 worldViewProjectionMatrix;
	    bool isUseSetWorld;

	
	public:

	void Initialize(PrimitiveName name);
	void Update();
	void Draw();
};
