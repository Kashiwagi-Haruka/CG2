#include "FileManager.h"
#include"FileObject.h"
#include "Object3d/Object3dCommon.h"

namespace {
    const uint32_t maxFile = 14;
}

FileManager::FileManager()
{
    for (int i = 0; i < maxFile; ++i) {
       std::unique_ptr<FileObject>newFile =  std::make_unique<FileObject>();
       newFile->SetEditorRegistrationName("FileObject" + std::to_string(i));


       files_.push_back(std::move(newFile));
    }
}

void FileManager::Initialize()
{
    for (auto& file : files_) {
        file->Initialize();
    }
}

void FileManager::Update()
{
    for (auto& file : files_) {
        file->Update();
    }
}

void FileManager::Draw()
{
    Object3dCommon::GetInstance()->DrawCommon();
    for (auto& file : files_) {
        file->Draw();
    }
}

void FileManager::SetCamera(Camera* camera)
{
    for (auto& file : files_) {
        file->SetCamera(camera);
    }
}
