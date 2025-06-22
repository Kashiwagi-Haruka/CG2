#pragma once
#include <Windows.h>
#include <cstdint>
#include <format>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <cassert>
#include "ConvertString.h"
#include <dxcapi.h>
#include "Function.h"
#include "Vector4.h"
#include "imGuiM.h"
#include "Texture.h"
#include "VertexData.h"
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lparam);

class GameBase {

private:

	WNDCLASS wc{};
	RECT wrc;
	
	MSG msg{};
	ConvertString* CStr{};
	std::wstring wstringValue = L"k";

	ID3D12Debug1* debugController = nullptr;
	IDXGIFactory7* dxgiFactory;
	HRESULT hr_;
	HWND hwnd;
	// 使用するアダプタ用の変数。最初にnullptrを入れておく
	IDXGIAdapter4* useAdapter = nullptr;
	ID3D12Device* device_ = nullptr;
	ID3D12CommandQueue* commandQueue_;
	D3D12_COMMAND_QUEUE_DESC commandQueueDesc;
	ID3D12CommandAllocator* commandAllocator;
	ID3D12GraphicsCommandList* commandList_;
	IDXGISwapChain4* swapChain_ = nullptr;
	ID3D12DescriptorHeap* rtvDescriptorHeap_ = nullptr;
	imGuiM imguiM_;
	ID3D12DescriptorHeap* srvDescriptorHeap_ = nullptr;

	DXGI_SWAP_CHAIN_DESC1 swapChainDesc;
	// SwapChainからResourceを引っ張ってくる。
	ID3D12Resource* swapChainResources_[2] = {nullptr};
	// これから書き込むバックバッファのインデックスを取得
	UINT backBufferIndex_;
	// transitionBarrierの設定
	D3D12_RESOURCE_BARRIER barrier{};

	// 初期値0でFenceをつくる
	ID3D12Fence* fence_ = nullptr;
	uint64_t fenceValue_ = 0;
	// FenceのSignalを持つためのイベントを作成する
	HANDLE fenceEvent_ = CreateEvent(NULL, FALSE, FALSE, NULL);

	std::vector<Texture> textures_;

	int32_t kClientWidth = 1280;
	int32_t kClientHeight = 720;


	D3D12_VIEWPORT viewport;
	// シザー矩形
	D3D12_RECT scissorRect;
	ID3D12RootSignature* rootSignature;
	// 実際に生成
	ID3D12PipelineState* graphicsPipelineState;
	// 頂点バッファビューを作成する
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView_;

	// シリアライズしてバイナリにする
	ID3DBlob* signatureBlob;
	ID3DBlob* errorBlob;
	IDxcBlob* pixelShaderBlob;
	IDxcBlob* vertexShaderBlob;
	IDxcUtils* dxcUtils;
	IDxcCompiler3* dxcCompiler;
	IDxcIncludeHandler* includeHandler;

		// バッファの設定
	D3D12_HEAP_PROPERTIES heapProperties;
	D3D12_RESOURCE_DESC resourceDesc;
	// 実際に頂点リソースを作る
	ID3D12Resource* vertexResource_;
	ID3D12Resource* materialResource_;
	// --- 頂点用 (Transform行列) のリソース追加 ---
	ID3D12Resource* transformResource_ = nullptr;
	ID3D12Resource* vertexResourceSprite;
	ID3D12Resource* transformationMatrixResourceSprite;
	ID3D12Resource* vertexResourceSphere;

		// RTVを2つ作るのでディスクリプタを2つ用意
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles[2];
	ID3D12Resource* depthStenicilResource=nullptr;
	ID3D12DescriptorHeap* dsvDescriptorHeap;
	Function function;

	struct Material{

		Vector4 color;
		int enableLighting;
		float padding[3];
		Matrix4x4 uvTransform;
	};

	ID3D12Resource* materialResourceSprite_;
	Material materialDataSprite_;

	struct DirectionalLight{
		Vector4 color;
		Vector3 direction;
		float intensity;
	};

	ID3D12Resource* directionalLightResource_ = nullptr;
	DirectionalLight* directionalLightData_ = nullptr;


	struct TransformationMatrix {
		Matrix4x4 WVP;
		Matrix4x4 World;
	
	};

	struct Transform {

		Vector3 scale;
		Vector3 rotate;
		Vector3 translate;


	};

	Transform transform = {
	    {1.0f, 1.0f, 1.0f}, // scale
	    {0.0f, 0.0f, 0.0f}, // rotate
	    {0.0f, 0.0f, 0.0f}  // translate
	};

	Transform uvTransformSprite_{
	    {1.0f, 1.0f, 1.0f},
	    {0.0f, 0.0f, 0.0f},
	    {0.0f, 0.0f, 0.0f},
	};

Transform cameraTransform = {
	    {1.0f, 1.0f,       1.0f }, // スケール
	    {0.0f, 0, 0.0f }, // ←Y軸180度回転！
	    {0.0f, 0.0f,-10.0f}  // Zマイナス方向に下げる
	};
	Transform transformSprite{
	    {1.0f, 1.0f, 1.0f},
        {0.0f, 0.0f, 0.0f},
        {0.0f, 0.0f, 0.0f}
    };
struct MaterialData {
	std::string textureFilePath;
};
struct ModelData {
	std::vector<VertexData> vertices;
	MaterialData material;
};
ModelData modelData;


	ID3D12Resource* indexResourceSprite_ = nullptr;
	D3D12_INDEX_BUFFER_VIEW indexBufferViewSprite_{};
	Matrix4x4* transformationMatrixDataSprite;
	D3D12_VERTEX_BUFFER_VIEW vertexBufferViewSprite;
	
D3D12_VERTEX_BUFFER_VIEW vertexBufferViewSphere;
	int kVertexCount_;

	//Matrix4x4* wvpData = nullptr; // ← transformResource用のポインタをメンバに持つ
	Matrix4x4* transformationMatrixData = nullptr;

	Texture texture_;
	Texture texture2_;
	D3D12_GPU_DESCRIPTOR_HANDLE GPUHandle_;
	D3D12_GPU_DESCRIPTOR_HANDLE GPUHandle2_;
	
	UINT currentTriangleVertexOffset_ = 0;  // 追加：次に書き込む頂点のオフセット（頂点数単位）
	UINT currentSphereVertexOffset_ = 0;
	UINT currentSpriteVertexOffset_ = 0;
	static constexpr UINT kMaxVertices_ = 1024; // 十分な大きさで定義しておく


	   bool useMonsterBall_ = true;

	void FrameStart(); // フレーム最初の準備
	   


   public:	

	void BeginFlame(); // フレームの開始処理（commandListリセットなど）
	void EndFlame();   // フレームの終了処理（Present、フェンス待ちなど）
	void Initialize(const wchar_t* TitleName, int32_t WindowWidth, int32_t WindowHeight);

	bool IsMsgQuit();

	void OutPutLog();

	static LONG WINAPI ExportDump(EXCEPTION_POINTERS* exception);

	MSG* GetMsg() { return &msg; };
	void Update();

	void CheackResourceLeaks();

	void ResourceRelease();

	// 描画関数（好きなだけ呼べるように）
	void DrawTriangle(const Vector3 positions[3], const Vector2 texcoords[3], const Vector4& color, int textureHandle);
	void DrawSphere(const Vector3& center, float radius, uint32_t color, int textureHandle);
	void DrawSprite(int texHandle, const Vector2& pos, float scale, float rotate, uint32_t color, int textureHandle);
	ModelData LoadObjFile(const std::string& directoryPath, const std::string& filename);
	MaterialData LoadMaterialTemplateFile(const std::string& directoryPath, const std::string& filename);

private:

	static LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

	void Log(const std::string& message);

	void WindowClear();

	void DebugLayer();
	void DebugError();
	void TransitionBarrier();

	void CrtvTransitionBarrier();

	void FenceEvent();



	void DXCInitialize();
	IDxcBlob* CompileShader(// CompilerするShaderファイルへのパス
	    const std::wstring& filePath,
	    // Compilerに使用するProfile
	    const wchar_t* profile,
	    // 初期化で生成したものをつかう
	    IDxcUtils* dxcUtils, IDxcCompiler3* dxcCompiler, IDxcIncludeHandler* includeHandler);
	void PSO();

	void VertexResource();

	void DrawCommandList();
	void CreateModelVertexBuffer();
	void CreateSpriteVertexBuffer();
	ID3D12Resource* CreateBufferResource(ID3D12Device* device_, size_t sizeInBytes);
	ID3D12DescriptorHeap* CreateDescriptorHeap(ID3D12Device* device_, D3D12_DESCRIPTOR_HEAP_TYPE heapType, UINT numDescriptors, bool shaderVisible);
	ID3D12Resource* CreateDepthStencilTextureResource(ID3D12Device* device_, int32_t width, int32_t height);

	

	int LoadTexture(const std::string& fileName);
	
};

