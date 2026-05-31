#include "FileManager.h"
#include"FileObject.h"


FileManager::FileManager(const uint32_t count)
{
    for (uint32_t i = 0; i < count; ++i) {
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
