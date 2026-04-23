#include "DocumentManager.h"

DocumentManager::DocumentManager()
{
    documentParticle_ = std::make_unique<DocumentParticle>();
    document_ = std::make_unique<Document>();
}

void DocumentManager::Initialize()
{
    documentParticle_->Initialize();
    documentParticle_->SetEmitArea({ -1.0f, 0.0f, -1.0f }, { 1.0f, 0.0f, 1.0f });
    // 基準点を上空(Y = 10.0f)に設定し、0.2秒ごとに2枚ずつ降らせる
    documentParticle_->StartEmit({ 4.0f, 4.0f, 0.0f }, 0.2f, 2);
    document_->Initialize();
    document_->SetTransform({ { 1.0f,1.0f,1.0f }, { 0.0f,1.0f,0.0f }, {4.0f,0.03f,0.0f} });
}

void DocumentManager::Draw()
{

    documentParticle_->Draw();
    document_->Draw();
}

void DocumentManager::Update(Camera* camera, const Vector3& lightDirection)
{

    documentParticle_->Update(camera, lightDirection);
    document_->Update();
}
