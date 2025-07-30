#include "ObjDraw.h"

#include <numeric> // std::iota 用

void ObjDraw::Initialize(GameBase& gameBase) {
	sprite.transform.scale = {0.5f, 0.5f, 1.0f};
	sprite.transform.rotate = {0.0f, 0.0f, 0.0f};
	sprite.transform.translate = {-0.5f,0.5f, 0.0f};
	sprite.texturePos[0] = {0.0f, 0.0f}; // 左上
	sprite.texturePos[1] = {1.0f, 0.0f}; // 右上
	sprite.texturePos[2] = {1.0f, 1.0f}; // 右下
	sprite.texturePos[3] = {0.0f, 1.0f}; // 左下
	sprite.UVTransform.scale = {1.0f, 1.0f, 1.0f}; // UVのスケール
	sprite.UVTransform.rotate = {0.0f, 0.0f, 0.0f}; // UVの回転
	sprite.UVTransform.translate = {0.0f, 0.0f, 0.0f}; // UVの移動
	sprite.worldTexturePos[0] = {sprite.texturePos[0].x, sprite.texturePos[0].y, 0.0f};
	sprite.worldTexturePos[1] = {sprite.texturePos[1].x, sprite.texturePos[1].y, 0.0f};
	sprite.worldTexturePos[2] = {sprite.texturePos[2].x, sprite.texturePos[2].y, 0.0f};
	sprite.worldTexturePos[3] = {sprite.texturePos[3].x, sprite.texturePos[3].y, 0.0f};
	sphere.transform.scale = {10.0f, 10.0f, 10.0f};
	sphere.transform.rotate = {0.0f, 0.0f, 0.0f};
	sphere.transform.translate = {50.0f, 50.0f, 0.0f};
	sphere.directionalLight.color = {1.0f, 1.0f, 1.0f, 1.0f};
	sphere.directionalLight.direction = {1.0f, -1.0f, -1.0f};
	sphere.directionalLight.intensity = 1.5f;
	planeModel = gameBase.GetPlaneModelData();
	for (int i = 0; i < planeModel.vertices.size(); ++i) {
		// 頂点の位置を調整
		planeModel.vertices[i].position.z = 5.0f; // X軸方向に-0.5f
		planeModel.vertices[i].position.x += 50.0f;
		planeModel.vertices[i].position.y += 50.0f; // Y軸方向に+0.5f
	}
	// ObjDraw::Initialize
	object.transform.scale = {10.0f, 10.0f, 10.0f};
	object.transform.rotate = {0.0f, 0.0f, 0.0f};
	object.transform.translate = {0.0f, 0.0f, 0.0f};
}

void ObjDraw::DrawObjSprite(GameBase& gameBase, const Matrix4x4 viewProj){

		 

		

	Matrix4x4 uvTransformMatrix = fn.MakeIdentity();
	uvTransformMatrix = fn.MakeScaleMatrix(sprite.UVTransform.scale);
	uvTransformMatrix = fn.Multiply(uvTransformMatrix, fn.MakeRotateZMatrix(sprite.UVTransform.rotate.z));
	uvTransformMatrix = fn.Multiply(uvTransformMatrix, fn.MakeTranslateMatrix(sprite.UVTransform.translate));
	sprite.worldTexturePos[0] = fn.Transform(sprite.texturePos[0], uvTransformMatrix);
	sprite.worldTexturePos[1] = fn.Transform(sprite.texturePos[1], uvTransformMatrix);
	sprite.worldTexturePos[2] = fn.Transform(sprite.texturePos[2], uvTransformMatrix);
	sprite.worldTexturePos[3] = fn.Transform(sprite.texturePos[3], uvTransformMatrix);
	sprite.UV[0] = {sprite.worldTexturePos[0].x, sprite.worldTexturePos[0].y};
	sprite.UV[1] = {sprite.worldTexturePos[1].x, sprite.worldTexturePos[1].y};
	sprite.UV[2] = {sprite.worldTexturePos[2].x, sprite.worldTexturePos[2].y};
	sprite.UV[3] = {sprite.worldTexturePos[3].x, sprite.worldTexturePos[3].y};

	Vector3 spritePos[4]{};
	
	spritePos[0] = {
	    sprite.transform.translate.x - sprite.transform.scale.x, sprite.transform.translate.y + sprite.transform.scale.y,
	    sprite.transform.translate.z};
	spritePos[1] =
	{
	    sprite.transform.translate.x + sprite.transform.scale.x, sprite.transform.translate.y + sprite.transform.scale.y,
	    sprite.transform.translate.z};
	spritePos[2] = {
	    sprite.transform.translate.x + sprite.transform.scale.x, sprite.transform.translate.y -  sprite.transform.scale.y,
	    sprite.transform.translate.z};
	spritePos[3] = {
	    sprite.transform.translate.x - sprite.transform.scale.x, sprite.transform.translate.y - sprite.transform.scale.y,
	    sprite.transform.translate.z};
	/*gameBase.DrawSpriteSheet(spritePos, sprite.UV, 0xffffffff);*/






	
		// --- (ImGuiまわりやUV計算は省略・今のまま維持でOK) ---

		// 1. ワールド行列を作成
		Matrix4x4 S = fn.MakeScaleMatrix(object.transform.scale);
		Matrix4x4 R = fn.MakeRotateXMatrix(object.transform.rotate.x);
		R = fn.Multiply(R, fn.MakeRotateYMatrix(object.transform.rotate.y));
		R = fn.Multiply(R, fn.MakeRotateZMatrix(object.transform.rotate.z));
		Matrix4x4 T = fn.MakeTranslateMatrix(object.transform.translate);
		Matrix4x4 world = fn.Multiply(S, fn.Multiply(R, T));

		// 2. ビュー行列
		auto cam = gameBase.GetCameraTransform();
		Matrix4x4 camMat = fn.MakeAffineMatrix(cam.scale, cam.rotate, cam.translate);
		Matrix4x4 view = fn.Inverse(camMat);

		// 3. 投影行列
		Matrix4x4 proj = fn.MakePerspectiveFovMatrix(0.45f, 1280.0f / 720.0f, 0.1f, 100.0f);

		// 4. 最終 WVP
		Matrix4x4 wvp = fn.Multiply(world, fn.Multiply(view, proj));
		gameBase.SetTransformMatrixWVP(wvp, 2);

		// 5. plane.objの頂点・インデックス取得
		ModelData md = gameBase.GetPlaneModelData();
		std::vector<uint32_t> indices;
		if (!md.indices.empty()) {
			indices = md.indices;
		} else {
			// もしobjのindicesが空なら [0,1,2,3,4,5] を作成
			indices.resize(md.vertices.size());
			std::iota(indices.begin(), indices.end(), 0);
		}

		// 6. インデックス付きでメッシュ描画
		gameBase.DrawMesh(md.vertices, indices, 0xffffffff, 1); // uvChecker.png: tex=1

		// --- ImGuiデバッグ出力（省略せずに残してOK） ---
		ImGui::Begin("ObjSprite");
		ImGui::Text("Sprite");
		ImGui::DragFloat3("Translate spr", &sprite.transform.translate.x, 0.1f);
		ImGui::DragFloat3("Rotate spr", &sprite.transform.rotate.x, 0.1f);
		ImGui::DragFloat3("Scale spr", &sprite.transform.scale.x, 0.1f, 0.5f);
		ImGui::Separator();
		ImGui::Text("TexturePos");
		ImGui::DragFloat3("Translate tex", &sprite.UVTransform.translate.x, 0.01f, -10.0f, 10.0f);
		ImGui::DragFloat3("Scale tex", &sprite.UVTransform.scale.x, 0.01f, -10.0f, 10.0f);
		ImGui::SliderAngle("Rotate tex", &sprite.UVTransform.rotate.z);
		ImGui::Separator();
		ImGui::Text("Object");
		ImGui::DragFloat3("Translate obj", &object.transform.translate.x, 0.1f);
		ImGui::DragFloat3("Rotate obj", &object.transform.rotate.x, 0.1f);
		ImGui::DragFloat3("Scale obj", &object.transform.scale.x, 0.1f, 0.5f);
		ImGui::Text("plane vertices: %d", (int)planeModel.vertices.size());
		for (int i = 0; i < planeModel.vertices.size(); ++i) {
			ImGui::Text("Vertex %d: (%f, %f, %f)", i, md.vertices[i].position.x, md.vertices[i].position.y, md.vertices[i].position.z);
			ImGui::Text("Normal %d: (%f, %f, %f)", i, planeModel.vertices[i].normal.x, planeModel.vertices[i].normal.y, planeModel.vertices[i].normal.z);
			ImGui::Text("Texcoord %d: (%f, %f)", i, planeModel.vertices[i].texcoord.x, planeModel.vertices[i].texcoord.y);
		}
		ImGui::End();
	}


void ObjDraw::DrawSphere(GameBase& gameBase, const Matrix4x4 viewProj){

	ImGui::Begin("Sphere");
	if (ImGui::Button("RESET")) {
		sphere.transform.scale = {10.0f, 10.0f, 10.0f};
		sphere.transform.rotate = {0.0f, 0.0f, 0.0f};
		sphere.transform.translate = {50.0f, 50.0f, 0.0f};
		sphere.directionalLight.color = {1.0f, 1.0f, 1.0f, 1.0f};
		sphere.directionalLight.direction = {1.0f, -1.0f, -1.0f};
		sphere.directionalLight.intensity = 1.5f;
	}
	if (ImGui::TreeNode("Transform")) {
	
	ImGui::DragFloat3("Translate", &sphere.transform.translate.x, 0.1f);
	ImGui::DragFloat3("Rotate", &sphere.transform.rotate.x, 0.1f);
	ImGui::DragFloat3("Scale", &sphere.transform.scale.x, 0.1f,1.0f);
	ImGui::TreePop();
	}
	ImGui::Separator();

	if (ImGui::TreeNode("DirectionalLight")) {
	
	ImGui::ColorEdit4("Color", &sphere.directionalLight.color.x);
	ImGui::DragFloat3("Direction", &sphere.directionalLight.direction.x, 0.1f);
	ImGui::DragFloat("Intensity", &sphere.directionalLight.intensity, 0.1f, 0.0f, 10.0f);
	ImGui::TreePop();
	}
	ImGui::End();
	
	gameBase.DrawSphere(sphere.transform.translate, sphere.transform.scale, sphere.transform.rotate,0xffffffff, -1,viewProj);

}