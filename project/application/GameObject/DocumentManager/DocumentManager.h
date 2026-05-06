#pragma once
#include<memory>
#include"Vector3.h"
#include"DocumentParticle/DocumentParticle.h"
#include"Document/Document.h"
#include"DocumentSprite/DocumentSprite.h"
class Camera;
class PlayerCamera;

class DocumentManager
{
private:
    std::unique_ptr<DocumentParticle>  documentParticle_ = nullptr;
    std::unique_ptr<Document>  document_ = nullptr;
    std::unique_ptr<DocumentSprite>  documentSprite_ = nullptr;


public:
    DocumentManager();
    void Initialize();
    void Draw();
    void DrawSprite();
    void Update(Camera* camera, const Vector3& lightDirection);
    void SetPlayerCamera(PlayerCamera* playerCamera) { document_->SetPlayerCamera(playerCamera); };
    void SetCamera(Camera* camera) { document_->SetCamera(camera); };
};

