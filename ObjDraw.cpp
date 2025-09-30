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

	// ObjDraw::Initialize
	object.transform.scale = {1.0f, 1.0f, 1.0f};
	object.transform.rotate = {0.0f, 0.0f, 0.0f};
	object.transform.translate = {0.0f, 0.0f, 0.0f};
	gameBase.directionalLightData_->direction = {0.0f, -1.0f, 0.0f}; // ライトの方向
	gameBase.directionalLightData_->color = {1.0f, 1.0f, 1.0f, 1.0f}; // ライトの色
	gameBase.directionalLightData_->intensity = 1.0f;                 // ライトの強度
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
	gameBase.DrawSpriteSheet(spritePos, sprite.UV, 0xffffffff);

	// 1) ワールド行列を作成
	Matrix4x4 S = fn.MakeScaleMatrix(object.transform.scale);
	Matrix4x4 R = fn.MakeRotateXMatrix(object.transform.rotate.x);
	R = fn.Multiply(R, fn.MakeRotateYMatrix(object.transform.rotate.y));
	R = fn.Multiply(R, fn.MakeRotateZMatrix(object.transform.rotate.z));
	Matrix4x4 T = fn.MakeTranslateMatrix(object.transform.translate);
	Matrix4x4 world = fn.Multiply(S, fn.Multiply(R, T));

	// 2) viewProj は already 「ビュー×射影」が来ている想定なので
	//    world * viewProj で最終 WVP を得る
	Matrix4x4 wvp = fn.Multiply(world, viewProj);

	// 3) スロット 1（VertexShader用 CBV）に書き込む
	//    ※ GameBase 側でスロット番号は適宜合わせてください
	gameBase.SetTransformMatrixWVP(wvp, 1);

	// 4) ObjDraw::Initialize で取得しておいた ModelData を再取得
	//    （planeModel は Initialize() で gameBase.GetPlaneModelData() してあるはず）
	const auto& md = planeModel;

	// 5) DrawMesh を呼んで頂点＋テクスチャを一発描画
	//    引数は (頂点リスト, RGBA カラー, テクスチャスロット番号)
	gameBase.DrawMesh(md.vertices, 0xffffffff, /*uvCheckerを１番スロットにロードしている*/ 1,wvp,world);

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
	


	ImGui::End();
	
	gameBase.DrawSphere(sphere.transform.translate, sphere.transform.scale, sphere.transform.rotate,0xffffffff, -1,viewProj);

}

void ObjDraw::HarfLightControl(GameBase& gameBase) {
	if (!gameBase.GetDirectionalLightData())
		return;

	
    
    
	ImGui::Begin("Half Lambert Light");

	ImGui::ColorEdit3("Light Color", &gameBase.directionalLightData_->color.x);
	ImGui::DragFloat3("Light Direction", &gameBase.directionalLightData_->direction.x, 0.01f, -1.0f, 1.0f);
	ImGui::DragFloat("Intensity", &gameBase.directionalLightData_->intensity, 0.01f, 0.0f, 10.0f);

	if (ImGui::Button("RESET")) {
		gameBase.directionalLightData_->color = {1.0f, 1.0f, 1.0f, 1.0f};
		gameBase.directionalLightData_->direction = {0.0f, -1.0f, 0.0f};
		gameBase.directionalLightData_->intensity = 1.0f;
	}

	// 方向ベクトルの正規化
	Vector3& dir = gameBase.directionalLightData_->direction;
	float len = sqrtf(dir.x * dir.x + dir.y * dir.y + dir.z * dir.z);
	if (len > 0.0001f) {
		dir.x /= len;
		dir.y /= len;
		dir.z /= len;
	}

	ImGui::End();
}
