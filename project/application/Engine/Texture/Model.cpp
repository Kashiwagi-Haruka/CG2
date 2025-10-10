//#include "Model.h"
//#include "WinApp.h"
//#include "Matrix4x4.h"
//void Model::CreateModelVertexBuffer() {
//	
//	// 頂点リソース作成
//
//	const int MaxVertexCount = 655360;
//	vertexResource_ = CreateBufferResource(device_.Get(), sizeof(VertexData) * MaxVertexCount);
//
//	vertexBufferView_.BufferLocation = vertexResource_->GetGPUVirtualAddress();
//	vertexBufferView_.SizeInBytes = UINT(sizeof(VertexData) * modelData.vertices.size());
//	vertexBufferView_.StrideInBytes = sizeof(VertexData);
//
//	VertexData* vertexData = nullptr;
//	vertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
//	std::memcpy(vertexData, modelData.vertices.data(), sizeof(VertexData) * modelData.vertices.size());
//
//	// ビューポートとシザー設定
//	viewport = {};
//	viewport.Width = float(WinApp::kClientWidth);
//	viewport.Height = float(WinApp::kClientHeight);
//	viewport.TopLeftX = 0;
//	viewport.TopLeftY = 0;
//	viewport.MinDepth = 0.0f;
//	viewport.MaxDepth = 1.0f;
//
//	scissorRect = {};
//	scissorRect.left = 0;
//	scissorRect.right = WinApp::kClientWidth;
//	scissorRect.top = 0;
//	scissorRect.bottom = WinApp::kClientHeight;
//
//	// --- マテリアル用リソース ---
//	// 3D用（球など陰影つけたいもの）
//	// 必ず256バイト単位で切り上げる
//	size_t alignedSize = (sizeof(Material) + 0xFF) & ~0xFF;
//	materialResource_ = CreateBufferResource(device_.Get(), alignedSize);
//	Material* mat3d = nullptr;
//	materialResource_->Map(0, nullptr, reinterpret_cast<void**>(&mat3d));
//	mat3d->color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
//	mat3d->enableLighting = 1;
//	mat3d->uvTransform = function.MakeIdentity();
//
//	materialResource_->Unmap(0, nullptr);
//
//	// --- トランスフォーム用リソース ---
//
//	constexpr UINT kCBAlign = 256;
//	UINT matrixAlignedSize = (sizeof(TransformationMatrix) + kCBAlign - 1) & ~(kCBAlign - 1);
//	transformResource_ = CreateBufferResource(device_.Get(), matrixAlignedSize * kMaxTransformSlots);
//
//	transformResource_->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrixData)); // 構造体配列で取得
//	                                                                                          // ←ここ！！起動時にマップしっぱなし
//	                                                                                          // すべて単位行列で初期化（任意）
//	for (int i = 0; i < kMaxTransformSlots; ++i) {
//		transformationMatrixData[i].WVP = function.MakeIdentity();
//		transformationMatrixData[i].World = function.MakeIdentity();
//	} // 初期値は単位行列
//	Matrix4x4 worldMatrix = function.MakeAffineMatrix(transform.scale, transform.rotate, transform.translate);
//
//	Matrix4x4 cameraMatrix = function.MakeAffineMatrix(cameraTransform.scale, cameraTransform.rotate, cameraTransform.translate);
//	Matrix4x4 viewMatrix = function.Inverse(cameraMatrix);
//	// ProjectionMatrixの作成（FOV: 0.45f * π、アスペクト比: 16:9、近クリップ: 0.1f、遠クリップ: 1000.0f）
//	float fovY = 0.45f * 3.14159265f; // ラジアン
//	float aspectRatio = 16.0f / 9.0f;
//	float nearClip = 0.1f;
//	float farClip = 1000.0f;
//
//	Matrix4x4 projectionMatrix = function.MakePerspectiveFovMatrix(fovY, aspectRatio, nearClip, farClip);
//	Matrix4x4 worldViewProjectionMatrix = function.Multiply(worldMatrix, function.Multiply(viewMatrix, projectionMatrix));
//
//}