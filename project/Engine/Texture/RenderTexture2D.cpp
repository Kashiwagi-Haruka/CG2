#include "RenderTexture2D.h"
#include "DirectXCommon.h"
#include "SrvManager/SrvManager.h"
#include "TextureManager.h"
#include "Object3d/Object3dCommon.h"
#include <cassert>

void RenderTexture2D::Initialize(uint32_t width, uint32_t height, DXGI_FORMAT format, const std::array<float, 4>& clearColor) {
	width_ = width;
	height_ = height;
	dxCommon_ = Object3dCommon::GetInstance()->GetDxCommon();
	format_ = format;
	clearColor_ = clearColor;

	if (!dxCommon_ || !TextureManager::GetInstance()) {
		initialized_ = false;
		return;
	}

	D3D12_RESOURCE_DESC textureDesc{};
	textureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	textureDesc.Width = width;
	textureDesc.Height = height;
	textureDesc.DepthOrArraySize = 1;
	textureDesc.MipLevels = 1;
	textureDesc.Format = format_;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	textureDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

	D3D12_HEAP_PROPERTIES heapProps{};
	heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;

	D3D12_CLEAR_VALUE clearValue{};
	clearValue.Format = format_;
	clearValue.Color[0] = clearColor_[0];
	clearValue.Color[1] = clearColor_[1];
	clearValue.Color[2] = clearColor_[2];
	clearValue.Color[3] = clearColor_[3];

	HRESULT hr = dxCommon_->GetDevice()->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &textureDesc, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, &clearValue, IID_PPV_ARGS(&resource_));
	if (FAILED(hr)) {
		initialized_ = false;
		return;
	}

	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc{};
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvHeapDesc.NumDescriptors = 1;
	hr = dxCommon_->GetDevice()->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&rtvHeap_));
	if (FAILED(hr)) {
		initialized_ = false;
		return;
	}

	rtvHandle_ = rtvHeap_->GetCPUDescriptorHandleForHeapStart();
	dxCommon_->GetDevice()->CreateRenderTargetView(resource_.Get(), nullptr, rtvHandle_);

	srvIndex_ = TextureManager::GetInstance()->GetSrvManager()->Allocate();
	TextureManager::GetInstance()->GetSrvManager()->CreateSRVforTexture2D(srvIndex_, resource_.Get(), format_, 1);

	currentState_ = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
	initialized_ = true;
}

void RenderTexture2D::TransitionToRenderTarget(ID3D12GraphicsCommandList* commandList) {
	if (!initialized_ || !commandList || currentState_ == D3D12_RESOURCE_STATE_RENDER_TARGET) {
		return;
	}
	D3D12_RESOURCE_BARRIER barrier{};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Transition.pResource = resource_.Get();
	barrier.Transition.StateBefore = currentState_;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	commandList->ResourceBarrier(1, &barrier);
	currentState_ = D3D12_RESOURCE_STATE_RENDER_TARGET;
}

void RenderTexture2D::TransitionToShaderResource(ID3D12GraphicsCommandList* commandList) {
	if (!initialized_ || !commandList || currentState_ == D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE) {
		return;
	}
	D3D12_RESOURCE_BARRIER barrier{};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Transition.pResource = resource_.Get();
	barrier.Transition.StateBefore = currentState_;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	commandList->ResourceBarrier(1, &barrier);
	currentState_ = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
}

void RenderTexture2D::BeginRender(ID3D12GraphicsCommandList* commandList) {
	if (!initialized_ || !commandList) {
		return;
	}
	TransitionToRenderTarget(commandList);
	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = dxCommon_->GetDepthStencilViewHandle();
	commandList->OMSetRenderTargets(1, &rtvHandle_, false, &dsvHandle);

	D3D12_VIEWPORT viewport{};
	viewport.TopLeftX = 0.0f;
	viewport.TopLeftY = 0.0f;
	viewport.Width = static_cast<float>(width_);
	viewport.Height = static_cast<float>(height_);
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	commandList->RSSetViewports(1, &viewport);

	D3D12_RECT scissorRect{};
	scissorRect.left = 0;
	scissorRect.top = 0;
	scissorRect.right = static_cast<LONG>(width_);
	scissorRect.bottom = static_cast<LONG>(height_);
	commandList->RSSetScissorRects(1, &scissorRect);

	commandList->ClearRenderTargetView(rtvHandle_, clearColor_.data(), 0, nullptr);
	commandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
}