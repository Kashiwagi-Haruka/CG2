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
protected:
    std::unique_ptr<Document>  document_ = nullptr;
    std::unique_ptr<DocumentSprite>  documentSprite_ = nullptr;
public:
    DocumentManager();
    virtual void Initialize(const std::string name);
    virtual void Draw();
    virtual void Update(Camera* camera, const Vector3& lightDirection);
    void DrawSprite();
    void SetPlayerCamera(PlayerCamera* playerCamera) { document_->SetPlayerCamera(playerCamera); };
    void SetCamera(Camera* camera) { document_->SetCamera(camera); };
};

class DocumentManagerParticle :public DocumentManager
{
private:
    std::unique_ptr<DocumentParticle>  documentParticle_ = nullptr;
public:
    DocumentManagerParticle();
    void Initialize(const std::string name)override;
    void Draw()override;
    void Update(Camera* camera, const Vector3& lightDirection)override;
};
