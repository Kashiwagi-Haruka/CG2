#pragma once
#include<memory>
#include<vector>
#include"FileObject.h"

class Camera;

class FileObject;

class FileManager
{
public:
    FileManager();
    void Initialize();
    void Update();
    void Draw();
    void SetCamera(Camera* camera);
private:
    std::vector<std::unique_ptr<FileObject>>files_;
};

