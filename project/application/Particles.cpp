#include "Particles.h"
#include "ParticleManager.h"
#include "Function.h"
#include "imgui.h"
Particles::Particles(){

	ParticleManager::GetInstance()->CreateParticleGroup("player", "Resources/2d/defaultParticle.png");
	ParticleManager::GetInstance()->CreateParticleGroup("leaf", "Resources/2d/leaf.png");
	ParticleManager::GetInstance()->CreateParticleGroup("goal", "Resources/2d/goalParticle.png");
	ParticleManager::GetInstance()->CreateParticleGroup("screenEffect", "Resources/2d/defaultParticle.png");
	particleplayer = new ParticleEmitter(
	    "player",
	    {
	        {0.1f, 0.1f, 1.0f},
            {0.0f, 0.0f, 0.0f},
            {0,    0,    0   }
    },
	    1, 5, {-0.01f, 0.001f, 0}, {-10.0f, 0, -1}, {10.0f, 3, 0});
	particleleaf = new ParticleEmitter(
	    "leaf",
	    {
	        {0.5f, 0.5f, 1.0f},
            {0.0f, 0.0f, 0.0f},
            {0.0f, 0.0f, 0.0f}
    },
	    1, 5, {0.0f, -0.1f, 0}, {-640, 0, -1}, {640, 320, 0});
	particlegoal = new ParticleEmitter(
	    "goal",
	    {
	        {0.2f, 0.2f, 1.0f},
            {0.0f, 0.0f, 0.0f},
            {0,    0,    0   }
    },
	    1, 5, {0, 0.1f, 0}, {-10.0f, 0, -1}, {10.0f, 3, 0});
	
}
Particles::~Particles(){

	
	delete particlegoal;
	delete particleleaf;
	delete particleplayer;



}
constexpr float kParticlePosScale = 5.0f; // プレイヤー→パーティクル座標系

void Particles::Update() {
	playerEmitterTransform.scale = {0.1f, 0.1f, 1.0f};
	playerEmitterTransform.rotate = {0, 0, 0};

	// プレイヤー位置をパーティクル座標系に合わせて変換
	playerEmitterTransform.translate = {
	    playerPos_.x * kParticlePosScale, (playerPos_.y-0.9f) * kParticlePosScale,
	    playerPos_.z // Z はそのままでもOKならそのまま
	};

	particleplayer->Update(playerEmitterTransform);


	particleleaf->Update({
	    {0.5f,         0.5f,         1.0f},
        {0,            0,            0   },
        {cameraPos_.x, cameraPos_.y, 0   }
    });

	particlegoal->Update({
	    {0.2f, 0.2f, 1.0f},
        {0.0f, 0.0f, 0.0f},
        goalPos_ * 2.5f
    });



	// ============================
	//   ★ IMGUI EDITOR ★
	// ============================
	if (ImGui::Begin("Particles Editor")) {

		// ------- player -------
		ImGui::Text("Player Emitter");
		EditSingleEmitter(particleplayer);

		// ------- leaf -------
		ImGui::Separator();
		ImGui::Text("Leaf Emitter");
		EditSingleEmitter(particleleaf);

		// ------- goal -------
		ImGui::Separator();
		ImGui::Text("Goal Emitter");
		EditSingleEmitter(particlegoal);

	}
	ImGui::End();
}
void Particles::SetPlayerPos(Vector3 playerPos){ playerPos_ = playerPos; }
void Particles::SetCameraPos(Vector3 cameraPos) { cameraPos_ = cameraPos; }
void Particles::SetGoalPos(Vector3 goalPos) { goalPos_ = goalPos; }
void Particles::EditSingleEmitter(ParticleEmitter* e) {

	ImGui::PushID(e);

	// ============================================
	//  Transform Editor
	// ============================================
	if (ImGui::TreeNode("Transform")) {

		Transform t = e->GetTransformRef();

		// --- Scale ---
		ImGui::Text("Scale");
		ImGui::DragFloat3("##scale", &t.scale.x, 0.01f);

		// --- Rotate ---
		ImGui::Text("Rotate");
		ImGui::DragFloat3("##rotate", &t.rotate.x, 0.01f);

		// --- Translate ---
		ImGui::Text("Translate");
		ImGui::DragFloat3("##translate", &t.translate.x, 0.01f);

		ImGui::TreePop();
	}

	// ============================================
	//  Frequency
	// ============================================
	float freq = e->GetFrequency();
	if (ImGui::DragFloat("Frequency##freq", &freq, 0.01f, 0.0f, 10.0f)) {
		e->SetFrequency(freq);
	}

	// ============================================
	//  Emit Count
	// ============================================
	int cnt = (int)e->GetCount();
	if (ImGui::DragInt("Emit Count##cnt", &cnt, 1, 1, 1000)) {
		e->SetCount((uint32_t)cnt);
	}

	// ============================================
	//  Acceleration
	// ============================================
	Vector3 acc = e->GetAcceleration();
	if (ImGui::DragFloat3("Acceleration##acc", &acc.x, 0.01f)) {
		e->SetAcceleration(acc);
	}

	// ============================================
	//  Area Min / Max
	// ============================================
	Vector3 amin = e->GetAreaMin();
	if (ImGui::DragFloat3("Area Min##amin", &amin.x, 0.01f)) {
		e->SetAreaMin(amin);
	}

	Vector3 amax = e->GetAreaMax();
	if (ImGui::DragFloat3("Area Max##amax", &amax.x, 0.01f)) {
		e->SetAreaMax(amax);
	}

	if (ImGui::Button("Emit Now##emit")) {
		e->Emit();
	}

	ImGui::PopID();
}
