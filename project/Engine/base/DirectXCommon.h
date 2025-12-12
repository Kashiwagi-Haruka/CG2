#pragma once
#include <Windows.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>

// DXC (DirectX Shader Compiler)
#include <dxcapi.h>

// SAL マクロ用
#include <sal.h>

#include <array>
#include <string>
#include <vector>
#include <chrono>

#include "WinApp.h"

#include "Vector4.h"
#include "Vector3.h"
#include "Vector2.h"
#include "VertexData.h"
#include "Matrix4x4.h"
#include "Function.h"
#include "BlendModeManeger.h"
#include "Transform.h"
#include "Light.h"

struct MaterialData {
	std::string textureFilePath;
};
struct ModelData {
	std::vector<VertexData> vertices;
	MaterialData material;
};

class BlendModeManeger;
class SrvManager;

class DirectXCommon {

	HRESULT hr_;
	Microsoft::WRL::ComPtr<ID3D12Device> device_ = nullptr;
	Microsoft::WRL::ComPtr<IDXGIFactory7> dxgiFactory_;
	Microsoft::WRL::ComPtr<IDXGIAdapter4> useAdapter_ = nullptr;
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue_;
	D3D12_COMMAND_QUEUE_DESC commandQueueDesc_;
	
	static const UINT kFrameCount = 2; // スワップチェインのバッファ数に合わせる
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> commandAllocators_[kFrameCount];
	UINT frameIndex_ = 0;

	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList_;
	Microsoft::WRL::ComPtr<IDXGISwapChain4> swapChain_ = nullptr;
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc_;
	Microsoft::WRL::ComPtr<ID3D12Debug1> debugController_ = nullptr;
	// これから書き込むバックバッファのインデックスを取得
	UINT backBufferIndex_;
	uint32_t descriptorSizeSRV_ = 0;
	uint32_t descriptorSizeRTV_ = 0;
	uint32_t descriptorSizeDSV_ = 0;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> rtvDescriptorHeap_ = nullptr;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> srvDescriptorHeap_ = nullptr;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> dsvDescriptorHeap_ = nullptr;
	std::array<Microsoft::WRL::ComPtr<ID3D12Resource>, 2> swapChainResources_;


	// RTVの設定
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc_{};
	// RTVを2つ作るのでディスクリプタを2つ用意
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles_[2];

	Microsoft::WRL::ComPtr<IDxcUtils> dxcUtils_;
	Microsoft::WRL::ComPtr<IDxcCompiler3> dxcCompiler_;
	Microsoft::WRL::ComPtr<IDxcIncludeHandler> includeHandler_;

	Microsoft::WRL::ComPtr<ID3D12Resource> depthStenicilResource_ = nullptr;

	// 初期値0でFenceをつくる
	Microsoft::WRL::ComPtr<ID3D12Fence> fence_ = nullptr;
	uint64_t fenceValue_ = 0;
	// FenceのSignalを持つためのイベントを作成する
	HANDLE fenceEvent_;

	D3D12_VIEWPORT viewport_;
	// シザー矩形
	D3D12_RECT scissorRect_;
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_;
	Microsoft::WRL::ComPtr<ID3D12RootSignature> particleRootSignature_;
	
		// パーティクル用

	// 実際に生成
	Microsoft::WRL::ComPtr<ID3D12PipelineState> graphicsPipelineState_[6];
	Microsoft::WRL::ComPtr<ID3D12PipelineState> particlePipelineState_[BlendMode::kCountOfBlendMode];
	Microsoft::WRL::ComPtr<ID3D12PipelineState> graphicsPipelineStateWhite_; // 完全白用
	// 頂点バッファビューを作成する
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView_;
	// transitionBarrierの設定
	D3D12_RESOURCE_BARRIER barrier_{};
	// シリアライズしてバイナリにする
	Microsoft::WRL::ComPtr<ID3DBlob> signatureBlob_;
	Microsoft::WRL::ComPtr<ID3DBlob> errorBlob_;
	Microsoft::WRL::ComPtr<IDxcBlob> pixelShaderBlob_;
	Microsoft::WRL::ComPtr<IDxcBlob> vertexShaderBlob_;
	// 実際に頂点リソースを作る
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource_;
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource_;
	// --- 頂点用 (Transform行列) のリソース追加 ---
	Microsoft::WRL::ComPtr<ID3D12Resource> transformResource_ = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> directionalLightResource_ = nullptr;
	DirectionalLight* directionalLightData_ = nullptr;
	UINT currentTriangleVertexOffset_ = 0; // 追加：次に書き込む頂点のオフセット（頂点数単位）
	UINT currentSphereVertexOffset_ = 0;
	UINT currentSpriteVertexOffset_ = 0;
	int sphereDrawCallCount_ = 0;

	D3D12_VERTEX_BUFFER_VIEW vertexBufferViewSphere_;
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResourceSphere_;
	int kVertexCount_;
	const int kMaxSpheres = 30; // 複数球体用の最大数（例）

	

	Microsoft::WRL::ComPtr<ID3D12Resource> transformationMatrixResource_;

	
	


	struct Material {

		Vector4 color;
		int enableLighting;
		float padding[3];
		Matrix4x4 uvTransform;
	};



	// GameBase.h
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResourceMesh_;
	Microsoft::WRL::ComPtr<ID3D12Resource> indexResourceMetaball_;
	D3D12_VERTEX_BUFFER_VIEW vertexBufferViewMesh_;
	D3D12_INDEX_BUFFER_VIEW indexBufferViewMetaball_;

	// GameBase.h
	struct alignas(256) TransformationMatrix {
		Matrix4x4 WVP;   // 64 バイト
		Matrix4x4 World; // 64 バイト
		                 // ここで自動的に 128 バイト分のパディングが入って、
		                 // sizeof(TransformationMatrix) == 256 になる
	};
	struct TransformationMatrix128 {
		Matrix4x4 WVP;   // 64 バイト
		Matrix4x4 World; // 64 バイト
		                 
		                 
	};
	// 例: 最大確保サイズ（初期化時に使った値を定数などで保持しておく）
	const size_t kMaxVertexCount = 200000;    // 実際の最大確保頂点数
	const size_t kMaxIndexCount = 200000 * 3; // 実際の最大確保インデックス数

	Transform cameraTransform = {
	    {1.0f, 1.0f, 1.0f  }, // スケール
	    {0.0f, 0,    0.0f  }, // 
	    {0.0f, 0.0f, -10.0f}  //
	};

	// Matrix4x4* wvpData = nullptr; // ← transformResource用のポインタをメンバに持つ
	TransformationMatrix* transformationMatrixData = nullptr;

	// Texture texture_;
	// Texture texture2_;
	D3D12_GPU_DESCRIPTOR_HANDLE TextureGPUHandle_[10000];
	D3D12_GPU_DESCRIPTOR_HANDLE ModelGPUHandle_[10000];
	D3D12_CPU_DESCRIPTOR_HANDLE instancingSrvHandleCPU;
	D3D12_GPU_DESCRIPTOR_HANDLE instancingSrvHandleGPU;
	static constexpr UINT kMaxVertices_ = 1024; // 十分な大きさで定義しておく

	int transformSlotOffset = 0; // slot=0,1使用

	const int kMaxTransformSlots = 32; // 例えば最大32スロット（用途に合わせて）

	ModelData modelData_;


	BlendMode blendMode_ = BlendMode::kBlendModeAlpha;

	Transform transform = {
	    {1.0f, 1.0f, 1.0f}, // scale
	    {0.0f, 0.0f, 0.0f}, // rotate
	    {0.0f, 0.0f, 0.0f}  // translate
	};
	
	BlendModeManeger blendModeManeger_;
	WinApp* winApp_ = nullptr;
	

	std::chrono::steady_clock::time_point reference_;

	// Particle instancing用
	static const uint32_t kNumInstance = 10;
	Microsoft::WRL::ComPtr<ID3D12Resource> instancingResource_;
	TransformationMatrix* instancingData_ = nullptr;

	

public:

	

	void initialize(WinApp* winApp);
	
	void InitializeFixFPS();

	void UpdateFixFPS();
	void CreateInstancingSRV(SrvManager* srvManager);
	void PreDraw();
	void PostDraw();
	Microsoft::WRL::ComPtr<ID3D12Resource> CreateBufferResource(size_t sizeInBytes);
	void Finalize();

	ID3D12Device* GetDevice() { return device_.Get(); };
	ID3D12GraphicsCommandList* GetCommandList() { return commandList_.Get(); };
	ID3D12DescriptorHeap* GetSrvDescriptorHeap() { return srvDescriptorHeap_.Get(); };
	D3D12_RENDER_TARGET_VIEW_DESC GetRtvDesc() { return rtvDesc_; }
	
	D3D12_CPU_DESCRIPTOR_HANDLE GetSrvCpuDescriptorHandle(uint32_t index);
	D3D12_GPU_DESCRIPTOR_HANDLE GetSrvGpuDescriptorHandle(uint32_t index);
	void SetTextureGPUHanle(int index, D3D12_GPU_DESCRIPTOR_HANDLE handle) { TextureGPUHandle_[index] = handle; }
	void SetModelGPUHanle(int index, D3D12_GPU_DESCRIPTOR_HANDLE handle) { ModelGPUHandle_[index] = handle; }
	D3D12_GPU_DESCRIPTOR_HANDLE GetTextureGPUHanle(int index) { return TextureGPUHandle_[index]; }
	D3D12_GPU_DESCRIPTOR_HANDLE GetModelGPUHanle(int index) { return ModelGPUHandle_[index]; }
	Transform& GetCameraTransform() { return cameraTransform; }
	
	// DirectXCommon.h
	ID3D12RootSignature* GetParticleRootSignature() const { return particleRootSignature_.Get(); }
	ID3D12PipelineState* GetParticlePipelineState(BlendMode mode) const { return particlePipelineState_[static_cast<int>(mode)].Get(); }
	D3D12_GPU_VIRTUAL_ADDRESS GetMaterialResourceGPUVA() const { return materialResource_->GetGPUVirtualAddress(); }
	size_t GetSwapChainResourcesNum() const { return swapChainResources_.size(); }
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> GetCommandQueue() { return commandQueue_; }

	Microsoft::WRL::ComPtr<IDxcBlob> CompileShader( // CompilerするShaderファイルへのパス
	    const std::wstring& filePath,
	    // Compilerに使用するProfile
	    const wchar_t* profile);
	void SetDirectionalLightData(const DirectionalLight& directionalLight);

	void CreateSphereResources();

	/*void CreateModelResources();*/

	void VertexResource();

	void SetBlendMode(BlendMode blendmode);

	

	void DrawSphere(const Vector3& center, float radius, uint32_t color, int textureHandle, const Matrix4x4& viewProj);
	void DrawSphere(const Vector3& center, const Vector3& radius, const Vector3& rotation, uint32_t color, int textureHandle, const Matrix4x4& viewProj);
	

	void DrawMesh(const std::vector<VertexData>& vertices, uint32_t color, int textureHandle, const Matrix4x4& wvp, const Matrix4x4& world);
	void DrawParticle(const std::vector<VertexData>& vertices, uint32_t color, uint32_t textureHandle, const Matrix4x4& wvp, const Matrix4x4& world, int instanceCount);

	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE heapType, UINT numDescriptors, bool shaderVisible);

private:

	void DebugLayer();
	void DeviceInitialize();
	void DebugError();
	void CommandListInitialize();
	void SwapChainInitialize();
	void DepthBufferCreate();
	void DescriptorHeapCreate();
	
	void RenderTargetViewInitialize();
	void DepthStencilViewInitialize();
	Microsoft::WRL::ComPtr<ID3D12Resource> CreateDepthStencilTextureResource(ID3D12Device* device_, int32_t width, int32_t height);
	void FenceCreate();
	void ViewportRectInitialize();
	void ScissorRectInitialize();
	void DXCCompilerCreate();
	void ImGuiInitialize();

	void FrameStart(); // フレーム最初の準備
	void DrawCommandList(); // 描画コマンドリスト

	void CrtvTransitionBarrier();

	void SetupPSO();
	void SetupParticlePSO();
	
	
	static D3D12_GPU_DESCRIPTOR_HANDLE GetGpuDescriptorHandle(Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeap, uint32_t descriptorSize, uint32_t index);
	static D3D12_CPU_DESCRIPTOR_HANDLE GetCpuDescriptorHandle(Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeap, uint32_t descriptorSize, uint32_t index);
};
