#include "DocumentManager.h"

DocumentManager::DocumentManager()
{
    document_ = std::make_unique<Document>();
    documentSprite_ = std::make_unique<DocumentSprite>();
}
DocumentManagerParticle::DocumentManagerParticle()
{
    documentParticle_ = std::make_unique<DocumentParticle>();
    document_ = std::make_unique<Document>();
    documentSprite_ = std::make_unique<DocumentSprite>();

}

void DocumentManager::Initialize(const std::string name)
{
    //書類
    document_->Initialize(name);
      //スプライト
    documentSprite_->Initialize(name);
}
void DocumentManagerParticle::Initialize(const std::string name)
{
    documentParticle_->Initialize();
    documentParticle_->SetEmitArea({ -1.0f, 0.0f, -1.0f }, { 1.0f, 0.0f, 1.0f });
    // 基準点を上空(Y = 10.0f)に設定し、0.2秒ごとに2枚ずつ降らせる
    documentParticle_->StartEmit(document_->GetTranslate(), 0.2f, 2);
    document_->Initialize(name);
    //スプライト
    documentSprite_->Initialize(name);
}

void DocumentManager::Draw()
{
    document_->Draw();
}

void DocumentManagerParticle::Draw()
{
    documentParticle_->Draw();
    document_->Draw();
}

void DocumentManager::Update(Camera* camera, const Vector3& lightDirection)
{
    (void)lightDirection;
    document_->Update();

    if (document_->GetDocumentLook()) {
        documentSprite_->Update();
    }
}

void DocumentManagerParticle::Update(Camera* camera, const Vector3& lightDirection)
{
    document_->Update();
    documentParticle_->Update(camera, lightDirection);

    if (document_->GetDocumentLook()) {
        documentSprite_->Update();
    }
}

void DocumentManager::DrawSprite()
{
    if (document_->GetDocumentLook()) {
        documentSprite_->Draw();
    }
}


