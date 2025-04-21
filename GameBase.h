#pragma once
#include <Windows.h>
#include <cstdint>
#include <format>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <cassert>
#include "Vector4.h"
#include "ConvertString.h"
//dxc
#include <dxcapi.h>
#include"Function.h" 

class GameBase {

private:
	WNDCLASS wc{};
	RECT wrc;
	
	MSG msg{};
	ConvertString* CStr{};
	std::wstring wstringValue = L"k";

	ID3D12Debug1* debugController = nullptr;
	IDXGIFactory7* dxgiFactory;
	HRESULT hr;
	HWND hwnd;
	// 使用するアダプタ用の変数。最初にnullptrを入れておく
	IDXGIAdapter4* useAdapter = nullptr;
	ID3D12Device* device = nullptr;
	ID3D12CommandQueue* commandQueue;
	D3D12_COMMAND_QUEUE_DESC commandQueueDesc;
	ID3D12CommandAllocator* commandAllocator;
	ID3D12GraphicsCommandList* commandList;
	IDXGISwapChain4* swapChain = nullptr;
	ID3D12DescriptorHeap* rtvDescriptorHeap = nullptr;
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc;
	// SwapChainからResourceを引っ張ってくる。
	ID3D12Resource* swapChainResources[2] = {nullptr};
	// これから書き込むバックバッファのインデックスを取得
	UINT backBufferIndex;
	// transitionBarrierの設定
	D3D12_RESOURCE_BARRIER barrier{};
	// 実際に頂点リソースを作る
	
	// 初期値0でFenceをつくる
	ID3D12Fence* fence = nullptr;
	uint64_t fenceValue = 0;
	// FenceのSignalを持つためのイベントを作成する
	HANDLE fenceEvent = CreateEvent(NULL, FALSE, FALSE, NULL);


	int32_t kClientWidth = 1280;
	int32_t kClientHeight = 720;

	// DXC
	// dxcCompilerを初期化
	IDxcUtils* dxcUtils;
	IDxcCompiler3* dxcCompiler;

	IDxcIncludeHandler* includeHandler;
	IDxcBlobEncoding* ShaderSource;
	DxcBuffer shaderSourceBuffer;
	IDxcResult* shaderResult;
	IDxcBlob* shaderBlob;
	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{};
	D3D12_INPUT_ELEMENT_DESC inputElementDescs[1] = {};
	// BlendStateの設定
	D3D12_BLEND_DESC blendDesc{};
	// RasterizerStateの設定
	D3D12_RASTERIZER_DESC rasterizerDesc{};
	D3D12_ROOT_SIGNATURE_DESC descriptionRootsSignature{};
	// シリアライズしてバイナリにする
	ID3DBlob* signatureBlob;
	ID3DBlob* errorBlob;

	// バイナリを基に作成
	ID3D12RootSignature* rootSignature;



	//shaderをコンパイルする
	IDxcBlob* vertexShaderBlob;
	IDxcBlob* pixelShaderBlob;

		// 実際に生成
	ID3D12PipelineState* graphicsPipelineState;

	D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDesc{};

	ID3D12Resource* vertexResource;
	// 頂点バッファービューを生成する
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView{};
		// ビューポート
	D3D12_VIEWPORT viewport{};
	// シザー矩形
	D3D12_RECT scissorRect{};

	ID3D12Resource* materialResource;

	Function function;
	ID3D12Resource* wvpResource;
	ID3D12Resource* transformationMatrixResource;
	Matrix4x4* wvpData;
	Matrix4x4* transformationMatrixData;
	struct Transform {
		Vector3 scale;
		Vector3 rotate;
		Vector3 translate;
	};

	Transform transform;
	Transform cameraTransform = {
	    {1.0f, 1.0f, 1.0f},
        {0.0f, 0.0f, 0.0f},
        {0.0f,0.0f,-5.0f}
    };
	

public:

	

	static LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

	void Log(const std::string& message);

	void Initialize(const wchar_t* TitleName, int32_t WindowWidth, int32_t WindowHeight);

	void UpdateTransform();

	bool IsMsgQuit();

	void OutPutLog();

	static LONG WINAPI ExportDump(EXCEPTION_POINTERS* exception);

	void WindowClear();

	void DebugLayer();
	void DebugError();
	void TransitionBarrier();

	void CrtvTransitionBarrier();

	void FenceEvent();

	void CheackResourceLeaks();

	void ResourceRelease();

	MSG* GetMsg() { return &msg; };

	void DXCInitialize();
	/// <summary>
	/// CompileShader関数
	/// </summary>
	/// <param name="filePath">CompilerするShaderファイルへのパス</param>
	/// <param name="profile">Compilerに使用するProfile</param>
	/// <param name="dxcUtils">初期化で使用するものを3つ</param>
	/// <param name="dxcCompiler"></param>
	/// <param name="includeHandler"></param>
	/// <returns></returns>
	IDxcBlob* CompileShader(const std::wstring& filePath, const wchar_t* profile, IDxcUtils* dxcUtils, IDxcCompiler3* dxcCompiler, IDxcIncludeHandler* includeHandler);

	void RootSignature();
	 
	void InputLayout();
	void BlenderState();
	void RasterizerState();
	void SCompile();
	void PSO();

	void VertexResource();

	void VertexBufferView();
	void ResourceCommand();
	void TraiangleResourceRelease();

	ID3D12Resource* CreateBufferResource(ID3D12Device* device, size_t sizeInBytes);
	
	void MaterialResource();
	void TransformationMatrixResource();
};

