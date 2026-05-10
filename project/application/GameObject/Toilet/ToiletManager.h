#pragma once
#include "Toilet.h"
#include <vector>
#include <memory>

class ToiletManager
{
public:
    ToiletManager();
    static bool IsRayHit() { return isRayHit_; }
    void Initialize();
    void Update();
    void Draw();
    void SetCamera(Camera* camera);
    void SetPlayerCamera(PlayerCamera* playerCamera);
    std::vector<std::unique_ptr<Toilet>>& GetToilets() { return toilets_; };
private:
    // 必要に応じて生成する最大数を変更してください
    static constexpr uint32_t kMaxToilets_ = 20;
    std::vector<std::unique_ptr<Toilet>> toilets_;
    static bool isRayHit_;
};
