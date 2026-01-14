#include "Primitive.h"
#include "Camera.h"
#include "DirectXCommon.h"
#include "Function.h"
#include "Object3d/Object3dCommon.h"
#include "SrvManager/SrvManager.h"
#include "TextureManager.h"
#include <algorithm>
#include <cmath>
#include <cstring>

namespace {
constexpr float kPi = 3.14159265358979323846f;
constexpr uint32_t kDefaultSlices = 32;
constexpr uint32_t kDefaultStacks = 16;
constexpr float kHalfSize = 0.5f;

struct MeshData {
	std::vector<VertexData> vertices;
	std::vector<uint32_t> indices;
};

void AppendQuad(MeshData& mesh, const VertexData& v0, const VertexData& v1, const VertexData& v2, const VertexData& v3) {
	uint32_t baseIndex = static_cast<uint32_t>(mesh.vertices.size());
	mesh.vertices.push_back(v0);
	mesh.vertices.push_back(v1);
	mesh.vertices.push_back(v2);
	mesh.vertices.push_back(v3);
	mesh.indices.push_back(baseIndex + 0);
	mesh.indices.push_back(baseIndex + 2);
	mesh.indices.push_back(baseIndex + 1);
	mesh.indices.push_back(baseIndex + 2);
	mesh.indices.push_back(baseIndex + 3);
	mesh.indices.push_back(baseIndex + 1);
}

MeshData BuildPlane() {
	MeshData mesh;
	VertexData v0 = {
	    {-kHalfSize, -kHalfSize, 0.0f, 1.0f},
        {0.0f, 1.0f},
        {0.0f, 0.0f, -1.0f}
    };
	VertexData v1 = {
	    {kHalfSize, -kHalfSize, 0.0f, 1.0f},
        {1.0f, 1.0f},
        {0.0f, 0.0f, -1.0f}
    };
	VertexData v2 = {
	    {-kHalfSize, kHalfSize, 0.0f, 1.0f},
        {0.0f, 0.0f},
        {0.0f, 0.0f, -1.0f}
    };
	VertexData v3 = {
	    {kHalfSize, kHalfSize, 0.0f, 1.0f},
        {1.0f, 0.0f},
        {0.0f, 0.0f, -1.0f}
    };
	AppendQuad(mesh, v0, v1, v2, v3);
	return mesh;
}

MeshData BuildTriangle() {
	MeshData mesh;
	mesh.vertices = {
	    {{-kHalfSize, -kHalfSize, 0.0f, 1.0f}, {0.0f, 1.0f}, {0.0f, 0.0f, -1.0f}},
	    {{0.0f, kHalfSize, 0.0f, 1.0f},        {0.5f, 0.0f}, {0.0f, 0.0f, -1.0f}},
	    {{kHalfSize, -kHalfSize, 0.0f, 1.0f},  {1.0f, 1.0f}, {0.0f, 0.0f, -1.0f}},
	};
	mesh.indices = {0, 1, 2};
	return mesh;
}

MeshData BuildCircle(uint32_t slices) {
	MeshData mesh;
	mesh.vertices.reserve(slices + 1);
	mesh.indices.reserve(slices * 3);
	mesh.vertices.push_back({
	    {0.0f, 0.0f, 0.0f, 1.0f},
        {0.5f, 0.5f},
        {0.0f, 0.0f, -1.0f}
    });

	for (uint32_t i = 0; i <= slices; ++i) {
		float angle = (static_cast<float>(i) / static_cast<float>(slices)) * kPi * 2.0f;
		float x = std::cos(angle) * kHalfSize;
		float y = std::sin(angle) * kHalfSize;
		mesh.vertices.push_back({
		    {x, y, 0.0f, 1.0f},
            {(x / (kHalfSize * 2.0f)) + 0.5f, 0.5f - (y / (kHalfSize * 2.0f))},
            {0.0f, 0.0f, -1.0f}
        });
	}

	for (uint32_t i = 1; i <= slices; ++i) {
		mesh.indices.push_back(0);
		mesh.indices.push_back(i);
		mesh.indices.push_back(i + 1);
	}

	return mesh;
}

MeshData BuildRing(uint32_t slices) {
	MeshData mesh;
	const float innerRadius = kHalfSize * 0.6f;
	const float outerRadius = kHalfSize;
	mesh.vertices.reserve((slices + 1) * 2);
	mesh.indices.reserve(slices * 6);

	for (uint32_t i = 0; i <= slices; ++i) {
		float angle = (static_cast<float>(i) / static_cast<float>(slices)) * kPi * 2.0f;
		float cosA = std::cos(angle);
		float sinA = std::sin(angle);
		Vector3 normal = {0.0f, 0.0f, -1.0f};
		mesh.vertices.push_back({
		    {outerRadius * cosA, outerRadius * sinA, 0.0f, 1.0f},
            {0.5f + cosA * 0.5f, 0.5f - sinA * 0.5f},
            normal
        });
		mesh.vertices.push_back({
		    {innerRadius * cosA, innerRadius * sinA, 0.0f, 1.0f},
            {0.5f + cosA * 0.3f, 0.5f - sinA * 0.3f},
            normal
        });
	}

	for (uint32_t i = 0; i < slices; ++i) {
		uint32_t outer0 = i * 2;
		uint32_t inner0 = outer0 + 1;
		uint32_t outer1 = outer0 + 2;
		uint32_t inner1 = outer0 + 3;
		mesh.indices.push_back(outer0);
		mesh.indices.push_back(inner0);
		mesh.indices.push_back(outer1);
		mesh.indices.push_back(outer1);
		mesh.indices.push_back(inner0);
		mesh.indices.push_back(inner1);
	}

	return mesh;
}

MeshData BuildSphere(uint32_t slices, uint32_t stacks) {
	MeshData mesh;
	mesh.vertices.reserve((slices + 1) * (stacks + 1));
	mesh.indices.reserve(slices * stacks * 6);

	for (uint32_t y = 0; y <= stacks; ++y) {
		float v = static_cast<float>(y) / static_cast<float>(stacks);
		float theta = v * kPi;
		float sinTheta = std::sin(theta);
		float cosTheta = std::cos(theta);
		for (uint32_t x = 0; x <= slices; ++x) {
			float u = static_cast<float>(x) / static_cast<float>(slices);
			float phi = u * kPi * 2.0f;
			float sinPhi = std::sin(phi);
			float cosPhi = std::cos(phi);
			Vector3 normal = {sinTheta * cosPhi, cosTheta, sinTheta * sinPhi};
			Vector4 position = {normal.x * kHalfSize, normal.y * kHalfSize, normal.z * kHalfSize, 1.0f};
			mesh.vertices.push_back({
			    position, {u, 1.0f - v},
                 normal
            });
		}
	}

	for (uint32_t y = 0; y < stacks; ++y) {
		for (uint32_t x = 0; x < slices; ++x) {
			uint32_t i0 = y * (slices + 1) + x;
			uint32_t i1 = i0 + 1;
			uint32_t i2 = i0 + (slices + 1);
			uint32_t i3 = i2 + 1;
			mesh.indices.push_back(i0);
			mesh.indices.push_back(i2);
			mesh.indices.push_back(i1);
			mesh.indices.push_back(i1);
			mesh.indices.push_back(i2);
			mesh.indices.push_back(i3);
		}
	}

	return mesh;
}

MeshData BuildTorus(uint32_t slices, uint32_t stacks) {
	MeshData mesh;
	const float majorRadius = kHalfSize * 0.8f;
	const float minorRadius = kHalfSize * 0.3f;
	mesh.vertices.reserve((slices + 1) * (stacks + 1));
	mesh.indices.reserve(slices * stacks * 6);

	for (uint32_t y = 0; y <= stacks; ++y) {
		float v = static_cast<float>(y) / static_cast<float>(stacks);
		float theta = v * kPi * 2.0f;
		float cosTheta = std::cos(theta);
		float sinTheta = std::sin(theta);
		for (uint32_t x = 0; x <= slices; ++x) {
			float u = static_cast<float>(x) / static_cast<float>(slices);
			float phi = u * kPi * 2.0f;
			float cosPhi = std::cos(phi);
			float sinPhi = std::sin(phi);
			float radial = majorRadius + minorRadius * cosTheta;
			Vector3 normal = {cosPhi * cosTheta, sinTheta, sinPhi * cosTheta};
			Vector4 position = {radial * cosPhi, minorRadius * sinTheta, radial * sinPhi, 1.0f};
			mesh.vertices.push_back({
			    position, {u, 1.0f - v},
                 normal
            });
		}
	}

	for (uint32_t y = 0; y < stacks; ++y) {
		for (uint32_t x = 0; x < slices; ++x) {
			uint32_t i0 = y * (slices + 1) + x;
			uint32_t i1 = i0 + 1;
			uint32_t i2 = i0 + (slices + 1);
			uint32_t i3 = i2 + 1;
			mesh.indices.push_back(i0);
			mesh.indices.push_back(i2);
			mesh.indices.push_back(i1);
			mesh.indices.push_back(i1);
			mesh.indices.push_back(i2);
			mesh.indices.push_back(i3);
		}
	}

	return mesh;
}

MeshData BuildCylinder(uint32_t slices) {
	MeshData mesh;
	const float height = kHalfSize;
	const float radius = kHalfSize;

	for (uint32_t i = 0; i <= slices; ++i) {
		float u = static_cast<float>(i) / static_cast<float>(slices);
		float angle = u * kPi * 2.0f;
		float cosA = std::cos(angle);
		float sinA = std::sin(angle);
		Vector3 normal = {cosA, 0.0f, sinA};
		mesh.vertices.push_back({
		    {radius * cosA, -height, radius * sinA, 1.0f},
            {u, 1.0f},
            normal
        });
		mesh.vertices.push_back({
		    {radius * cosA, height, radius * sinA, 1.0f},
            {u, 0.0f},
            normal
        });
	}

	for (uint32_t i = 0; i < slices; ++i) {
		uint32_t base = i * 2;
		mesh.indices.push_back(base);
		mesh.indices.push_back(base + 1);
		mesh.indices.push_back(base + 2);
		mesh.indices.push_back(base + 2);
		mesh.indices.push_back(base + 1);
		mesh.indices.push_back(base + 3);
	}

	uint32_t baseIndex = static_cast<uint32_t>(mesh.vertices.size());
	mesh.vertices.push_back({
	    {0.0f, -height, 0.0f, 1.0f},
        {0.5f, 0.5f},
        {0.0f, -1.0f, 0.0f}
    });
	mesh.vertices.push_back({
	    {0.0f, height, 0.0f, 1.0f},
        {0.5f, 0.5f},
        {0.0f, 1.0f, 0.0f}
    });

	for (uint32_t i = 0; i <= slices; ++i) {
		float u = static_cast<float>(i) / static_cast<float>(slices);
		float angle = u * kPi * 2.0f;
		float cosA = std::cos(angle);
		float sinA = std::sin(angle);
		mesh.vertices.push_back({
		    {radius * cosA, -height, radius * sinA, 1.0f},
            {0.5f + cosA * 0.5f, 0.5f - sinA * 0.5f},
            {0.0f, -1.0f, 0.0f}
        });
		mesh.vertices.push_back({
		    {radius * cosA, height, radius * sinA, 1.0f},
            {0.5f + cosA * 0.5f, 0.5f - sinA * 0.5f},
            {0.0f, 1.0f, 0.0f}
        });
	}

	for (uint32_t i = 0; i < slices; ++i) {
		uint32_t bottom0 = baseIndex + 2 + i * 2;
		uint32_t bottom1 = bottom0 + 2;
		mesh.indices.push_back(baseIndex);
		mesh.indices.push_back(bottom1);
		mesh.indices.push_back(bottom0);

		uint32_t top0 = baseIndex + 3 + i * 2;
		uint32_t top1 = top0 + 2;
		mesh.indices.push_back(baseIndex + 1);
		mesh.indices.push_back(top0);
		mesh.indices.push_back(top1);
	}

	return mesh;
}

MeshData BuildCone(uint32_t slices) {
	MeshData mesh;
	const float height = kHalfSize;
	const float radius = kHalfSize;
	Vector4 tip = {0.0f, height, 0.0f, 1.0f};

	for (uint32_t i = 0; i <= slices; ++i) {
		float u = static_cast<float>(i) / static_cast<float>(slices);
		float angle = u * kPi * 2.0f;
		float cosA = std::cos(angle);
		float sinA = std::sin(angle);
		Vector3 normal = Function::Normalize({cosA, radius / height, sinA});
		mesh.vertices.push_back({
		    {radius * cosA, -height, radius * sinA, 1.0f},
            {u, 1.0f},
            normal
        });
		mesh.vertices.push_back({
		    tip, {u, 0.0f},
             normal
        });
	}

	for (uint32_t i = 0; i < slices; ++i) {
		uint32_t base = i * 2;
		mesh.indices.push_back(base);
		mesh.indices.push_back(base + 1);
		mesh.indices.push_back(base + 2);
	}

	uint32_t centerIndex = static_cast<uint32_t>(mesh.vertices.size());
	mesh.vertices.push_back({
	    {0.0f, -height, 0.0f, 1.0f},
        {0.5f, 0.5f},
        {0.0f, -1.0f, 0.0f}
    });
	for (uint32_t i = 0; i <= slices; ++i) {
		float u = static_cast<float>(i) / static_cast<float>(slices);
		float angle = u * kPi * 2.0f;
		float cosA = std::cos(angle);
		float sinA = std::sin(angle);
		mesh.vertices.push_back({
		    {radius * cosA, -height, radius * sinA, 1.0f},
            {0.5f + cosA * 0.5f, 0.5f - sinA * 0.5f},
            {0.0f, -1.0f, 0.0f}
        });
	}

	for (uint32_t i = 0; i < slices; ++i) {
		uint32_t ring0 = centerIndex + 1 + i;
		uint32_t ring1 = ring0 + 1;
		mesh.indices.push_back(centerIndex);
		mesh.indices.push_back(ring1);
		mesh.indices.push_back(ring0);
	}

	return mesh;
}

MeshData BuildBox() {
	MeshData mesh;
	const float s = kHalfSize;
	Vector3 normals[6] = {
	    {0.0f,  0.0f,  -1.0f},
        {0.0f,  0.0f,  1.0f },
        {-1.0f, 0.0f,  0.0f },
        {1.0f,  0.0f,  0.0f },
        {0.0f,  1.0f,  0.0f },
        {0.0f,  -1.0f, 0.0f },
	};

	Vector4 positions[6][4] = {
	    {{-s, -s, -s, 1.0f}, {s, -s, -s, 1.0f},  {-s, s, -s, 1.0f},  {s, s, -s, 1.0f} }, // front
	    {{s, -s, s, 1.0f},   {-s, -s, s, 1.0f},  {s, s, s, 1.0f},    {-s, s, s, 1.0f} }, // back
	    {{-s, -s, s, 1.0f},  {-s, -s, -s, 1.0f}, {-s, s, s, 1.0f},   {-s, s, -s, 1.0f}}, // left
	    {{s, -s, -s, 1.0f},  {s, -s, s, 1.0f},   {s, s, -s, 1.0f},   {s, s, s, 1.0f}  }, // right
	    {{-s, s, -s, 1.0f},  {s, s, -s, 1.0f},   {-s, s, s, 1.0f},   {s, s, s, 1.0f}  }, // top
	    {{-s, -s, s, 1.0f},  {s, -s, s, 1.0f},   {-s, -s, -s, 1.0f}, {s, -s, -s, 1.0f}}, // bottom
	};

	Vector2 uvs[4] = {
	    {0.0f, 1.0f},
        {1.0f, 1.0f},
        {0.0f, 0.0f},
        {1.0f, 0.0f}
    };

	for (int face = 0; face < 6; ++face) {
		VertexData v0 = {positions[face][0], uvs[0], normals[face]};
		VertexData v1 = {positions[face][1], uvs[1], normals[face]};
		VertexData v2 = {positions[face][2], uvs[2], normals[face]};
		VertexData v3 = {positions[face][3], uvs[3], normals[face]};
		AppendQuad(mesh, v0, v1, v2, v3);
	}

	return mesh;
}
} // namespace

void Primitive::Initialize(PrimitiveName name) {
	primitiveName_ = name;
	camera_ = Object3dCommon::GetInstance()->GetDefaultCamera();
	transformResource_ = Object3dCommon::GetInstance()->CreateBufferResource(sizeof(TransformationMatrix));
	cameraResource_ = Object3dCommon::GetInstance()->CreateBufferResource(sizeof(CameraForGpu));

	MeshData mesh;
	switch (primitiveName_) {
	case Primitive::Plane:
		mesh = BuildPlane();
		break;
	case Primitive::Circle:
		mesh = BuildCircle(kDefaultSlices);
		break;
	case Primitive::Ring:
		mesh = BuildRing(kDefaultSlices);
		break;
	case Primitive::Sphere:
		mesh = BuildSphere(kDefaultSlices, kDefaultStacks);
		break;
	case Primitive::Torus:
		mesh = BuildTorus(kDefaultSlices, kDefaultStacks);
		break;
	case Primitive::Cylinder:
		mesh = BuildCylinder(kDefaultSlices);
		break;
	case Primitive::Cone:
		mesh = BuildCone(kDefaultSlices);
		break;
	case Primitive::Triangle:
		mesh = BuildTriangle();
		break;
	case Primitive::Box:
		mesh = BuildBox();
		break;
	default:
		mesh = BuildPlane();
		break;
	}

	vertices_ = std::move(mesh.vertices);
	indices_ = std::move(mesh.indices);

	vertexResource_ = Object3dCommon::GetInstance()->CreateBufferResource(sizeof(VertexData) * vertices_.size());
	vertexBufferView_.BufferLocation = vertexResource_->GetGPUVirtualAddress();
	vertexBufferView_.SizeInBytes = static_cast<UINT>(sizeof(VertexData) * vertices_.size());
	vertexBufferView_.StrideInBytes = sizeof(VertexData);
	VertexData* vertexData = nullptr;
	vertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
	std::memcpy(vertexData, vertices_.data(), sizeof(VertexData) * vertices_.size());
	vertexResource_->Unmap(0, nullptr);

	indexResource_ = Object3dCommon::GetInstance()->CreateBufferResource(sizeof(uint32_t) * indices_.size());
	indexBufferView_.BufferLocation = indexResource_->GetGPUVirtualAddress();
	indexBufferView_.SizeInBytes = static_cast<UINT>(sizeof(uint32_t) * indices_.size());
	indexBufferView_.Format = DXGI_FORMAT_R32_UINT;
	void* mappedIndex = nullptr;
	indexResource_->Map(0, nullptr, &mappedIndex);
	std::memcpy(mappedIndex, indices_.data(), sizeof(uint32_t) * indices_.size());
	indexResource_->Unmap(0, nullptr);

	size_t alignedSize = (sizeof(Material) + 0xFF) & ~0xFF;
	materialResource_ = Object3dCommon::GetInstance()->CreateBufferResource(alignedSize);
	materialResource_->Map(0, nullptr, reinterpret_cast<void**>(&materialData_));
	materialData_->color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	materialData_->enableLighting = 1;
	materialData_->uvTransform = Function::MakeIdentity4x4();
	materialData_->shininess = 40.0f;
	materialData_->environmentCoefficient = 0.0f;
	materialResource_->Unmap(0, nullptr);

	const std::string texturePath = "Resources/3d/uvChecker.png";
	TextureManager::GetInstance()->LoadTextureName(texturePath);
	textureIndex_ = TextureManager::GetInstance()->GetTextureIndexByfilePath(texturePath);

	isUseSetWorld = false;
}

void Primitive::Update() {
	if (!isUseSetWorld) {
		worldMatrix = Function::MakeAffineMatrix(transform_.scale, transform_.rotate, transform_.translate);
	}

	worldViewProjectionMatrix = Function::Multiply(Function::Multiply(worldMatrix, camera_->GetViewMatrix()), camera_->GetProjectionMatrix());

	transformResource_->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrixData_));
	transformationMatrixData_->WVP = worldViewProjectionMatrix;
	transformationMatrixData_->World = worldMatrix;
	transformationMatrixData_->WorldInverseTranspose = Function::Inverse(worldMatrix);
	transformResource_->Unmap(0, nullptr);

	cameraResource_->Map(0, nullptr, reinterpret_cast<void**>(&cameraData_));
	if (camera_) {
		cameraData_->worldPosition = camera_->GetWorldTranslate();
	} else {
		cameraData_->worldPosition = {transform_.translate};
	}
	cameraResource_->Unmap(0, nullptr);
}

void Primitive::Draw() {
	ID3D12DescriptorHeap* descriptorHeaps[] = {TextureManager::GetInstance()->GetSrvManager()->GetDescriptorHeap().Get()};
	Object3dCommon::GetInstance()->GetDxCommon()->GetCommandList()->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

	Object3dCommon::GetInstance()->GetDxCommon()->GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferView_);
	Object3dCommon::GetInstance()->GetDxCommon()->GetCommandList()->IASetIndexBuffer(&indexBufferView_);

	Object3dCommon::GetInstance()->GetDxCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(0, materialResource_->GetGPUVirtualAddress());
	Object3dCommon::GetInstance()->GetDxCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(1, transformResource_->GetGPUVirtualAddress());
	Object3dCommon::GetInstance()->GetDxCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(3, Object3dCommon::GetInstance()->GetDirectionalLightResource()->GetGPUVirtualAddress());
	Object3dCommon::GetInstance()->GetDxCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(4, cameraResource_->GetGPUVirtualAddress());
	Object3dCommon::GetInstance()->GetDxCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(5, Object3dCommon::GetInstance()->GetPointLightCountResource()->GetGPUVirtualAddress());
	Object3dCommon::GetInstance()->GetDxCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(6, Object3dCommon::GetInstance()->GetSpotLightCountResource()->GetGPUVirtualAddress());
	Object3dCommon::GetInstance()->GetDxCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(7, Object3dCommon::GetInstance()->GetAreaLightCountResource()->GetGPUVirtualAddress());

	D3D12_GPU_DESCRIPTOR_HANDLE srvHandle = TextureManager::GetInstance()->GetSrvHandleGPU(textureIndex_);
	Object3dCommon::GetInstance()->GetDxCommon()->GetCommandList()->SetGraphicsRootDescriptorTable(2, srvHandle);
	TextureManager::GetInstance()->GetSrvManager()->SetGraphicsRootDescriptorTable(8, Object3dCommon::GetInstance()->GetPointLightSrvIndex());
	TextureManager::GetInstance()->GetSrvManager()->SetGraphicsRootDescriptorTable(9, Object3dCommon::GetInstance()->GetSpotLightSrvIndex());
	TextureManager::GetInstance()->GetSrvManager()->SetGraphicsRootDescriptorTable(10, Object3dCommon::GetInstance()->GetAreaLightSrvIndex());
	TextureManager::GetInstance()->GetSrvManager()->SetGraphicsRootDescriptorTable(11, Object3dCommon::GetInstance()->GetEnvironmentMapSrvIndex());

	Object3dCommon::GetInstance()->GetDxCommon()->GetCommandList()->DrawIndexedInstanced(static_cast<UINT>(indices_.size()), 1, 0, 0, 0);
}

void Primitive::SetCamera(Camera* camera) { camera_ = camera; }
void Primitive::SetTranslate(Vector3 translate) {
	transform_.translate = translate;
	isUseSetWorld = false;
}
void Primitive::SetRotate(Vector3 rotate) {
	transform_.rotate = rotate;
	isUseSetWorld = false;
}
void Primitive::SetScale(Vector3 scale) {
	transform_.scale = scale;
	isUseSetWorld = false;
}
void Primitive::SetTransform(Transform transform) {
	transform_ = transform;
	isUseSetWorld = false;
}
void Primitive::SetWorldMatrix(Matrix4x4 matrix) {
	worldMatrix = matrix;
	isUseSetWorld = true;
}
void Primitive::SetColor(Vector4 color) {
	materialResource_->Map(0, nullptr, reinterpret_cast<void**>(&materialData_));
	materialData_->color = color;
	materialResource_->Unmap(0, nullptr);
}
void Primitive::SetEnableLighting(bool enable) {
	materialResource_->Map(0, nullptr, reinterpret_cast<void**>(&materialData_));
	materialData_->enableLighting = enable ? 1 : 0;
	materialResource_->Unmap(0, nullptr);
}
void Primitive::SetUvTransform(const Matrix4x4& uvTransform) {
	materialResource_->Map(0, nullptr, reinterpret_cast<void**>(&materialData_));
	materialData_->uvTransform = uvTransform;
	materialResource_->Unmap(0, nullptr);
}
void Primitive::SetShininess(float shininess) {
	materialResource_->Map(0, nullptr, reinterpret_cast<void**>(&materialData_));
	materialData_->shininess = shininess;
	materialResource_->Unmap(0, nullptr);
}
void Primitive::SetEnvironmentCoefficient(float coefficient) {
	materialResource_->Map(0, nullptr, reinterpret_cast<void**>(&materialData_));
	materialData_->environmentCoefficient = coefficient;
	materialResource_->Unmap(0, nullptr);
}