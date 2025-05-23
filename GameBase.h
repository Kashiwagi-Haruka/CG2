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
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg_, WPARAM wParam, LPARAM lparam);

class GameBase {

private:

	WNDCLASS wc_{};
	RECT wrc_;
	
	MSG msg_{};
	ConvertString CStr_{};
	std::wstring wstringValue_ = L"k";

	ID3D12Debug1* debugController_ = nullptr;
	IDXGIFactory7* dxgiFactory_;
	HRESULT hr_;
	HWND hwnd_;
	// 使用するアダプタ用の変数。最初にnullptrを入れておく
	IDXGIAdapter4* useAdapter_ = nullptr;
	ID3D12Device* device_ = nullptr;
	ID3D12CommandQueue* commandQueue_;
	D3D12_COMMAND_QUEUE_DESC commandQueueDesc_;
	ID3D12CommandAllocator* commandAllocator_;
	ID3D12GraphicsCommandList* commandList_;
	IDXGISwapChain4* swapChain_ = nullptr;
	ID3D12DescriptorHeap* rtvDescriptorHeap_ = nullptr;
	imGuiM imguiM_;
	ID3D12DescriptorHeap* srvDescriptorHeap_ = nullptr;

	DXGI_SWAP_CHAIN_DESC1 swapChainDesc_;
	// SwapChainからResourceを引っ張ってくる。
	ID3D12Resource* swapChainResources_[2] = {nullptr};
	// これから書き込むバックバッファのインデックスを取得
	UINT backBufferIndex_;
	// transitionBarrierの設定
	D3D12_RESOURCE_BARRIER barrier_{};

	// 初期値0でFenceをつくる
	ID3D12Fence* fence_ = nullptr;
	uint64_t fenceValue_ = 0;
	// FenceのSignalを持つためのイベントを作成する
	HANDLE fenceEvent_ = nullptr;

	    // 新規：複数テクスチャ保持用コンテナ
	std::vector<Texture> textures_;

	int32_t kClientWidth_ = 1280;
	int32_t kClientHeight_ = 720;
	ID3D12Resource* bufferResource_ = nullptr;
	ID3D12Resource* resource_ = nullptr;
	D3D12_VIEWPORT viewport_;
	// シザー矩形
	D3D12_RECT scissorRect_;
	ID3D12RootSignature* rootSignature_;
	// 実際に生成
	ID3D12PipelineState* graphicsPipelineState_;
	// 頂点バッファビューを作成する
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView_;
	// 実際に頂点リソースを作る
	ID3D12Resource* vertexResource_;
	// シリアライズしてバイナリにする
	ID3DBlob* signatureBlob_;
	ID3DBlob* errorBlob_;
	IDxcBlob* pixelShaderBlob_;
	IDxcBlob* vertexShaderBlob_;
	IDxcUtils* dxcUtils_;
	IDxcCompiler3* dxcCompiler_;
	IDxcIncludeHandler* includeHandler_;

		// バッファの設定
	D3D12_HEAP_PROPERTIES heapProperties_;
	D3D12_RESOURCE_DESC resourceDesc_;
	ID3D12Resource* materialResource_;
	// --- 頂点用 (Transform行列) のリソース追加 ---
	ID3D12Resource* transformResource_ = nullptr;

		// RTVを2つ作るのでディスクリプタを2つ用意
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles_[2];
	ID3D12DescriptorHeap* dsvDescriptorHeap_;
	Function function_;

	struct Transform {

		Vector3 scale;
		Vector3 rotate;
		Vector3 translate;


	};

	Transform transform_ = {
	    {1.0f, 1.0f, 1.0f}, // scale
	    {0.0f, 0.0f, 0.0f}, // rotate
	    {0.0f, 0.0f, 0.0f}  // translate
	};

Transform cameraTransform_ = {
	    {1.0f, 1.0f,       1.0f }, // スケール
	    {0.0f, 0, 0.0f }, // ←Y軸180度回転！
	    {0.0f, 0.0f,-10.0f}  // Zマイナス方向に下げる
	};


	//Matrix4x4* wvpData = nullptr; // ← transformResource用のポインタをメンバに持つ
	Matrix4x4* transformationMatrixData_ = nullptr;

	Texture texture_;
	D3D12_GPU_DESCRIPTOR_HANDLE GPUHandle_;
	UINT currentVertexOffset_ = 0;             // 追加：次に書き込む頂点のオフセット（頂点数単位）
	static constexpr UINT kMaxVertices_ = 1024; // 十分な大きさで定義しておく

	ID3D12Resource* depthStencilResource_ = nullptr;

public:
	void FrameStart(); // フレーム最初の準備


	

	static LRESULT CALLBACK WindowProc(HWND hwnd_, UINT msg_, WPARAM wparam, LPARAM lparam);

	void Log(const std::string& message);

	void Initialize(const wchar_t* TitleName, int32_t WindowWidth, int32_t WindowHeight);

	void Update();
	/*void Draw();*/

	bool IsMsgQuit();

	void OutPutLog();

	static LONG WINAPI ExportDump(EXCEPTION_POINTERS* exception);

	void WindowClear();

	void DebugLayer();
	void DebugError();
	

	void CrtvTransitionBarrier();

	void FenceEvent();

	void CheackResourceLeaks();

	void ResourceRelease();

	MSG* GetMsg() { return &msg_; };

	int LoadTexture(const std::string& fileName);

	void DXCInitialize();
	IDxcBlob* CompileShader(// CompilerするShaderファイルへのパス
	    const std::wstring& filePath,
	    // Compilerに使用するProfile
	    const wchar_t* profile,
	    // 初期化で生成したものをつかう
	    IDxcUtils* dxcUtils_, IDxcCompiler3* dxcCompiler_, IDxcIncludeHandler* includeHandler_);
	void PSO();

	void VertexResource();

	void DrawCommandList();


	void BeginFlame();
	void EndFlame();
	
	void DrawTriangle(const Vector3 positions[3], const Vector2 texcoords[3], const Vector4& color, Texture& texture);
	
	  // テクスチャ取得用 (必要に応じて)
	Texture& GetTexture(int index) { return textures_.at(index); }/// <summary>GPU 定数バッファ（transformResource）に行列を書き込む</summary>
	void SetWorldViewProjection(const Matrix4x4& wvp);


	// GameBase.h に追記
	const Transform& GetCameraTransform() const { return cameraTransform_; }

	ID3D12Resource* CreateBufferResource(ID3D12Device* device_, size_t sizeInBytes);
	ID3D12DescriptorHeap* CreateDescriptorHeap(ID3D12Device* device_, D3D12_DESCRIPTOR_HEAP_TYPE heapType, UINT numDescriptors, bool shaderVisible);
	ID3D12Resource* CreateDepthStencilTextureResource(ID3D12Device* device_, int32_t width, int32_t height);

	ID3D12Device* GetDevice() { return device_; };
	ID3D12DescriptorHeap* GetSrvHeap() { return srvDescriptorHeap_; };
};

