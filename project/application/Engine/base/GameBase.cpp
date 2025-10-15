#define NOMINMAX
#include "GameBase.h"
#include <DbgHelp.h>
#include <strsafe.h>
#include <dxgidebug.h>
#include <fstream>
#include <sstream>

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "Dbghelp.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "dxcompiler.lib")

GameBase::~GameBase(){
	ResourceRelease();
	delete DInput;
	delete dxCommon_;
	delete winApp_;
}

void GameBase::Initialize(const wchar_t* TitleName, int32_t WindowWidth, int32_t WindowHeight) {

	winApp_ = new WinApp();
	winApp_->Initialize();

	dxCommon_ = new DirectXCommon();
	modelData = LoadObjFile("Resources/3d", "plane.obj");
	dxCommon_->SetModelData(modelData);
	dxCommon_->initialize(winApp_);

		
	
	DInput = new Input();
	DInput->Initialize(winApp_);

	audio.InitializeIXAudio();

}

bool GameBase::ProcessMessage() {

	return winApp_->ProcessMessage();
}


LONG WINAPI GameBase::ExportDump(EXCEPTION_POINTERS* exception) {
	// 時刻を取得して、時刻に名前を入れたファイルを作成。Dumpsディレクトリ以下に出力
	SYSTEMTIME time;
	GetLocalTime(&time);

	wchar_t filePath[MAX_PATH] = {0};
	CreateDirectory(L"./Dumps", nullptr);
	StringCchPrintfW(filePath, MAX_PATH, L"./Dumps/%04d-%02d%02d-%02d%02d.dmp", time.wYear, time.wMonth, time.wDay, time.wHour, time.wMinute);
	HANDLE dumpFileHandle = CreateFile(filePath, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_WRITE | FILE_SHARE_READ, 0, CREATE_ALWAYS, 0, 0);
	// processId(このexeのId)とクラッシュ(例外)の発生したthreadIdを取得
	DWORD processId = GetCurrentProcessId();
	DWORD threadId = GetCurrentThreadId();
	// 設定情報を入力
	MINIDUMP_EXCEPTION_INFORMATION mindumpInformation{0};
	mindumpInformation.ThreadId = threadId;
	mindumpInformation.ExceptionPointers = exception;
	mindumpInformation.ClientPointers = TRUE;
	// DUMPを出力。MiniDumpNormalは最低限の情報を出力するフラグ
	MiniDumpWriteDump(GetCurrentProcess(), processId, dumpFileHandle, MiniDumpNormal, &mindumpInformation, nullptr, nullptr);
	// 他に関連づけられてるSEH例外ハンドラがあれば実行。通常はプロセスを終了する。

	return EXCEPTION_EXECUTE_HANDLER;
}

void GameBase::CheackResourceLeaks() {

	Microsoft::WRL::ComPtr <IDXGIDebug1> debug;
	
	if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&debug)))) {

		debug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_ALL);
		debug->ReportLiveObjects(DXGI_DEBUG_APP, DXGI_DEBUG_RLO_ALL);
		debug->ReportLiveObjects(DXGI_DEBUG_D3D12, DXGI_DEBUG_RLO_ALL);
		debug->Release();
	}


}
void GameBase::ResourceRelease() {
	
	dxCommon_->Finalize();

	winApp_->Finalize();
}


void GameBase::SetDirectionalLightData(const DirectionalLight& directionalLight) { dxCommon_->SetDirectionalLightData(directionalLight); }



// 球体用リソース
void GameBase::BeginFlame() { 
	dxCommon_->PreDraw();
	DInput->Update();
}

// --- フレーム終了: ImGui 描画 → Present → フェンス同期まで ---
void GameBase::EndFlame() { dxCommon_->PostDraw(); }


//objfileを読む関数
ModelData GameBase::LoadObjFile(const std::string& directoryPath, const std::string& filename) {
	ModelData modelData;
	std::vector<Vector4> positions;
	std::vector<Vector3> normals;
	std::vector<Vector2> texcoords;
	std::string line;

	std::ifstream file(directoryPath + "/" + filename);
	assert(file.is_open());

	while (std::getline(file, line)) {
		std::string identifier;
		std::istringstream s(line);
		s >> identifier;

		if (identifier == "v") {
			Vector4 position;
			s >> position.x >> position.y >> position.z;
			position.w = 1.0f;
			// ここでx反転
			position.x *= -1.0f;
			
			positions.push_back(position);
		} else if (identifier == "vt") {
			Vector2 texcoord;
			s >> texcoord.x >> texcoord.y;
			texcoord.y=1.0f-texcoord.y;
			texcoords.push_back(texcoord);
		} else if (identifier == "vn") {
			Vector3 normal;
			s >> normal.x >> normal.y >> normal.z;
			// ここで法線もx反転
			
			normals.push_back(normal);
		} else if (identifier == "f") {
			VertexData triangle[3];
			for (int32_t faceVertex = 0; faceVertex < 3; ++faceVertex) {
				std::string vertexDefinition;
				s >> vertexDefinition;
				std::istringstream v(vertexDefinition);
				uint32_t elementIndices[3];
				for (int32_t element = 0; element < 3; ++element) {
					std::string index;
					std::getline(v, index, '/');
					elementIndices[element] = std::stoi(index);
				}
				Vector4 position = positions[elementIndices[0] - 1];
				Vector2 texcoord = texcoords[elementIndices[1] - 1];
				Vector3 normal = normals[elementIndices[2] - 1];
				triangle[faceVertex] = {position, texcoord, normal};
			}
			// 回り順を逆にしてpush_back
			modelData.vertices.push_back(triangle[2]);
			modelData.vertices.push_back(triangle[1]);
			modelData.vertices.push_back(triangle[0]);
		} else if (identifier == "mtllib") {
			std::string mtlFile;
			s >> mtlFile;
			modelData.material = LoadMaterialTemplateFile(directoryPath, mtlFile);
		}

	}

	

	return modelData;
}

MaterialData GameBase::LoadMaterialTemplateFile(const std::string& directoryPath, const std::string& filename) {
	MaterialData matData;
	std::ifstream file(directoryPath + "/" + filename);
	std::string line;
	assert(file.is_open());
	while (std::getline(file, line)) {
		std::istringstream s(line);
		std::string identifier;
		s >> identifier;
		if (identifier == "map_Kd") {
			std::string textureFilePaths;
			s >> textureFilePaths;
			matData.textureFilePath = directoryPath + "/" + textureFilePaths;
			
		}
	}
	return matData;
}

SoundData GameBase::SoundLoadWave(const char* filename){

	return audio.SoundLoadWave(filename);

}
void GameBase::SoundUnload(SoundData* soundData){

	audio.SoundUnload(soundData);

}
void GameBase::SoundPlayWave(const SoundData& sounddata) {
	assert(audio.GetIXAudio2() != nullptr); // 安全のため追加
	audio.SoundPlayWave(audio.GetIXAudio2().Get(), sounddata);
}

bool GameBase::PushMouseButton(Input::MouseButton button) const { return DInput->PushMouseButton(button); }

bool GameBase::TriggerMouseButton(Input::MouseButton button) const { return DInput->TriggerMouseButton(button); }

float GameBase::GetMouseX() const { return DInput->GetMouseX(); };
float GameBase::GetMouseY() const { return DInput->GetMouseY(); };
Vector2 GameBase::GetMouseMove() const { return DInput->GetMouseMove(); };

//void GameBase::DrawMesh(const std::vector<VertexData>& vertices, const std::vector<uint32_t>& indices, uint32_t color, int textureHandle) {
//	//if (vertices.empty() || indices.empty())
//	//	return;
//
//	// 0. まずここでPSO切り替え
//	if (IsMetaBall_) {
//		commandList_->SetPipelineState(graphicsPipelineStateWhite.Get()); // 白単色
//	} else {
//		
//		commandList_->SetPipelineState(graphicsPipelineState[blendMode_].Get()); // 通常
//	}
//	vertexResourceMesh_ = CreateBufferResource(device_.Get(), sizeof(VertexData) * vertices.size());
//	vertexBufferViewMesh_.BufferLocation = vertexResourceMesh_->GetGPUVirtualAddress();
//	vertexBufferViewMesh_.SizeInBytes = sizeof(VertexData) * static_cast<UINT>(vertices.size());
//	vertexBufferViewMesh_.StrideInBytes = sizeof(VertexData);
//
//	indexResourceMetaball_ = CreateBufferResource(device_.Get(), sizeof(uint32_t) * indices.size());
//	indexBufferViewMetaball_.BufferLocation = indexResourceMetaball_->GetGPUVirtualAddress();
//	indexBufferViewMetaball_.SizeInBytes = sizeof(uint32_t) * static_cast<UINT>(indices.size());
//	indexBufferViewMetaball_.Format = DXGI_FORMAT_R32_UINT;
//
//
//	/*OutputDebugStringA(std::format("vertexBufferView_.SizeInBytes={} useVertex={} 1vertex={} bytes\n", vertexBufferViewMesh_.SizeInBytes, vertices.size(), sizeof(VertexData)).c_str());*/
//	assert(vertices.size() * sizeof(VertexData) <= vertexBufferViewMesh_.SizeInBytes);
//
//	// バッファオーバーチェック
//	if (vertices.size() > kMaxVertexCount) {
//		OutputDebugStringA("DrawMesh: 頂点バッファサイズ超過！\n");
//		assert(false && "DrawMesh: 頂点バッファサイズ超過！");
//		return;
//	}
//	if (indices.size() > kMaxIndexCount) {
//		OutputDebugStringA("DrawMesh: インデックスバッファサイズ超過！\n");
//		assert(false && "DrawMesh: インデックスバッファサイズ超過！");
//		return;
//	}
//
//
//	// 転送
//	{
//		VertexData* vtxData = nullptr;
//		vertexResourceMesh_->Map(0, nullptr, reinterpret_cast<void**>(&vtxData));
//		memcpy(vtxData, vertices.data(), sizeof(VertexData) * vertices.size());
//		vertexResourceMesh_->Unmap(0, nullptr);
//	}
//	{
//		uint32_t* idxData = nullptr;
//		indexResourceMetaball_->Map(0, nullptr, reinterpret_cast<void**>(&idxData));
//		memcpy(idxData, indices.data(), sizeof(uint32_t) * indices.size());
//		indexResourceMetaball_->Unmap(0, nullptr);
//	}
//
//
//	// 描画時
//	commandList_->IASetVertexBuffers(0, 1, &vertexBufferViewMesh_);
//	commandList_->IASetIndexBuffer(&indexBufferViewMetaball_);
//
//	commandList_->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
//
//	// シェーダやテクスチャは各自合わせてください
//	// マテリアルや行列リソースなどセット
//	// ここはDrawTriangleやDrawSphereの実装と同じでOK
//	ID3D12DescriptorHeap* heaps[] = {srvDescriptorHeap_.Get()};
//	commandList_->SetDescriptorHeaps(_countof(heaps), heaps);
//
//	commandList_->SetGraphicsRootConstantBufferView(0, materialResource_->GetGPUVirtualAddress());
//	commandList_->SetGraphicsRootConstantBufferView(1, transformResource_->GetGPUVirtualAddress());
//	commandList_->SetGraphicsRootDescriptorTable(2, TextureGPUHandle_[textureHandle]); // ここは用途に合わせて
//
//	commandList_->DrawIndexedInstanced(static_cast<UINT>(indices.size()), 1, 0, 0, 0);
//}




int GameBase::LoadTextures(const std::string& fileName) {

	int handles = TexInitialize(dxCommon_->GetDevice(), dxCommon_->GetSrvDescriptorHeap(), fileName);
	dxCommon_->SetTextureGPUHanle(GetTextureToTal() - 1, GetTexGpuHandle());
	assert(dxCommon_->GetTextureGPUHanle(GetTextureToTal()-1).ptr != 0);
	return handles;
};
int GameBase::ModelTextures(const std::string& fileName) {

	int handles = ModelTexInitialize(dxCommon_->GetDevice(), dxCommon_->GetSrvDescriptorHeap(), fileName);
	dxCommon_->SetModelGPUHanle(GetmodelTexTotal() - 1, GetModelGpuHandle());
	assert(dxCommon_->GetModelGPUHanle(GetmodelTexTotal()-1).ptr != 0);

	return handles;
};

void GameBase::SetBlendMode(BlendMode mode) { dxCommon_->SetBlendMode(mode); }

bool GameBase::PushKey(BYTE keyNumber){ return DInput->PushKey(keyNumber); }
bool GameBase::TriggerKey(BYTE keyNumber) { return DInput->TriggerKey(keyNumber); }
bool GameBase::PushButton(Input::PadButton button) { return DInput->PushButton(button); }
bool GameBase::TriggerButton(Input::PadButton button) { return DInput->TriggerButton(button); }
// ジョイスティック

float GameBase::GetJoyStickLX() const { return DInput->GetJoyStickLX(); };


float GameBase::GetJoyStickLY() const { return DInput->GetJoyStickLY(); };

Vector2 GameBase::GetJoyStickLXY() const { return DInput->GetJoyStickLXY(); };

float GameBase::GetJoyStickRX() const { return DInput->GetJoyStickRX(); };
float GameBase::GetJoyStickRY() const { return DInput->GetJoyStickRY(); };
Vector2 GameBase::GetJoyStickRXY() const { return DInput->GetJoyStickRXY(); };


/// <summary>
/// デッドゾーンの設定
/// </summary>
/// <param name="deadZone">初期値は0.2f</param>
void GameBase::SetDeadZone(float deadZone) { DInput->SetDeadZone(deadZone); };

void GameBase::DrawMesh(const std::vector<VertexData>& vertices, uint32_t color, int textureHandle, const Matrix4x4& wvp, const Matrix4x4& world){ 

	dxCommon_->DrawMesh(vertices, color, textureHandle, wvp, world);
}
void GameBase::DrawParticle(const std::vector<VertexData>& vertices, uint32_t color, int textureHandle, const Matrix4x4& wvp, const Matrix4x4& world, int instanceCount) {
	dxCommon_->DrawParticle(vertices, color, textureHandle, wvp, world, instanceCount);
}
void GameBase::DrawSphere(const Vector3& center, float radius, uint32_t color, int textureHandle, const Matrix4x4& viewProj) {
	dxCommon_->DrawSphere(center, radius, color, textureHandle, viewProj);
}
void GameBase::DrawSpriteSheet(Vector3 pos[4], Vector2 texturePos[4], int color, int textureHandle) { 
	dxCommon_->DrawSpriteSheet(pos, texturePos, color, textureHandle); 
}