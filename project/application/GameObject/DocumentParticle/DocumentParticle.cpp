#include "DocumentParticle.h"
#include "Model/ModelManager.h"
#include <cmath>
#include"GameBase.h"
#include <random> 


// ★ 追加：簡単な乱数生成用の便利関数
namespace {
    float GetRandomFloat(float min, float max) {
        static std::random_device rd;
        static std::mt19937 gen(rd());
        std::uniform_real_distribution<float> dis(min, max);
        return dis(gen);
    }
}
void DocumentParticle::SetEmitArea(const Vector3& areaMin, const Vector3& areaMax) {
    emitAreaMin_ = areaMin;
    emitAreaMax_ = areaMax;
}
DocumentParticle::DocumentParticle()
{

    instancedObject_ = std::make_unique<InstancedObject3d>();
}

void DocumentParticle::Initialize() {

    // 1. 書類の3Dモデルを読み込む (モデルパスは適宜変更してください)
    ModelManager::GetInstance()->LoadModel("Resources/TD3_3102/3d/document", "document");
    instancedObject_->Initialize("document");
    instancedObject_->SetInstanceCount(kMaxInstanceCount);

    instances_.resize(kMaxInstanceCount);
    for (uint32_t i = 0; i < kMaxInstanceCount; ++i) {
        instances_[i].isActive = false;
        instancedObject_->SetInstanceOffset(i, { 1.0f, -1000.0f, 0.0f }); // 画面外へ
        instancedObject_->SetInstanceScale(i, { 1.0f, 1.0f, 1.0f });
        instancedObject_->SetEnableLighting(false);
    }
}

void DocumentParticle::Emit(const Vector3& center, uint32_t count) {
    // 空いている配列を探して初期化する処理 (Coffeesの発生処理を簡略化)
    uint32_t emitCount = 0;
    for (uint32_t i = 0; i < kMaxInstanceCount && emitCount < count; ++i) {
        if (!instances_[i].isActive) {
            auto& doc = instances_[i];
            doc.isActive = true;

            // ★ 修正：設定したエリア内でランダムな座標（オフセット）を計算
            float offsetX = GetRandomFloat(emitAreaMin_.x, emitAreaMax_.x);
            float offsetY = GetRandomFloat(emitAreaMin_.y, emitAreaMax_.y);
            float offsetZ = GetRandomFloat(emitAreaMin_.z, emitAreaMax_.z);

            // 発生基準位置（center）にランダムなズレを足す
            doc.position = {
                center.x + offsetX,
                center.y + offsetY, // Yにも範囲を持たせたい場合はここでズレます
                center.z + offsetZ
            };

            doc.velocity = { 0.0f, -0.05f, 0.0f }; // 下方向への速度

            // ついでに初期の回転角度もランダムにしておくと自然になります

            float twoPi = Function::kPi * 2.0f;
            doc.rotation = {
                GetRandomFloat(0.0f,twoPi),
                GetRandomFloat(0.0f,twoPi),
                GetRandomFloat(0.0f, twoPi)
            };

            doc.rotationVelocity = { 0.05f, 0.05f, 0.05f }; // 回転速度
            doc.wobblePhase = GetRandomFloat(0.0f, twoPi); // 揺らぎの開始タイミングもランダムに
            doc.lifeTime = 0.0f;
            doc.maxLife = 10.0f;

            emitCount++;
        }
    }
}
void DocumentParticle::StartEmit(const Vector3& center, float interval, uint32_t countPerEmit) {
    isEmitting_ = true;
    emitCenter_ = center;
    emitInterval_ = interval;
    emitCount_ = countPerEmit;
    emitTimer_ = 0.0f; // タイマーをリセット
}

// --- 追加: 発生停止 ---
void DocumentParticle::StopEmit() {
    isEmitting_ = false;
}

void DocumentParticle::SetTexture()
{
    
}

void DocumentParticle::Update(Camera* camera, const Vector3& lightDirection) {
  
    // C++
    if (!instancedObject_) return;
    
    const float deltaTime = GameBase::GetInstance()->GetDeltaTime();

    // 1. 時間による自動発生処理
    if (isEmitting_) {
        emitTimer_ += deltaTime;
        // 指定した間隔（秒）を超えたら発生
        while (emitTimer_ >= emitInterval_) {
            Emit(emitCenter_, emitCount_);
            emitTimer_ -= emitInterval_; // タイマーを減らして正確な周期を保つ
        }
    }

    for (uint32_t i = 0; i < kMaxInstanceCount; ++i) {
        if (!instances_[i].isActive) {
            instancedObject_->SetInstanceOffset(i, { 0.0f, -1000.0f, 0.0f });
            continue;
        }

        auto& doc = instances_[i];
        doc.lifeTime += deltaTime;
        if (doc.lifeTime >= doc.maxLife) {
            doc.isActive = false;
            continue;
        }

        // まだ空中にいる場合
        if (doc.position.y > 0.0f) {
            // ひらひら挙動の計算
            doc.wobblePhase += 3.0f * deltaTime;
            doc.position.x += std::sin(doc.wobblePhase) * 0.02f;
            doc.position.z += std::cos(doc.wobblePhase * 0.8f) * 0.02f;

            // 空気抵抗と重力
            doc.velocity.y -= 9.8f * deltaTime * 0.05f;
            doc.velocity.y *= 0.95f;

            doc.position.x += doc.velocity.x;
            doc.position.y += doc.velocity.y;
            doc.position.z += doc.velocity.z;

            doc.rotation.x += doc.rotationVelocity.x;
            doc.rotation.y += doc.rotationVelocity.y;
            doc.rotation.z += doc.rotationVelocity.z;

            // ★ 床（Y = 0）に衝突した瞬間の処理
            if (doc.position.y <= 0.0f) {
                doc.position.y = 0.0f;                     // Y座標を床の高さに固定
                doc.velocity = { 0.0f, 0.0f, 0.0f };         // 落下・移動を完全に停止
                doc.rotationVelocity = { 0.0f, 0.0f, 0.0f }; // 回転を停止

                // ★ 平らになるようにX軸とZ軸の回転を0にする
                // ※ Y軸の回転(yaw)はそのまま残すことで、床に散らばった時の向きがランダムになります
                doc.rotation.x = 0.0f;
                doc.rotation.z = 0.0f;
            }
        }

        // --- InstancedObject3dへ反映 ---
        instancedObject_->SetInstanceOffset(i, doc.position);
        instancedObject_->SetInstanceRotate(i, doc.rotation);
    }

    instancedObject_->Update(camera, lightDirection);
}

void DocumentParticle::Draw() {
    instancedObject_->Draw();
}