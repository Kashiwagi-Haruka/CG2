#include "FileManager.h"

#include"FileObject.h"

namespace {
    const uint32_t maxFile = 3;
}

FileManager::FileManager()
{
    for (int i = 0; i < maxFile; ++i) {
       std::unique_ptr<FileObject>newFile =  std::make_unique<FileObject>();
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
