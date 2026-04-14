#pragma once
#include "Light/CommonLight/AreaCommonLight.h"
#include "Light/CommonLight/PointCommonLight.h"
#include "Light/CommonLight/SpotCommonLight.h"
#include "Light/CommonLight/DirectionalCommonLight.h" 

#include <array>
#include <memory>

namespace Yoshida {
    class LightManager
    {
    public:
        enum Lights {
            DIRECTIONAL,
            POINT,
            SPOT,
            AREA,
            MAX_LIGHT
        };

        void SetActiveLightCount(Lights lightType, uint32_t count);
        void SetPointLight(const PointCommonLight& pointLight,uint32_t index);
        void SetAreaLight(const AreaCommonLight& areaLight, uint32_t index);
        void SetSpotLight(const SpotCommonLight& spotLight, uint32_t index);
        void Initialize();
        void Update();
        void ClearLights();
        const DirectionalCommonLight& GetDirectionalLight() { return directionalLight_; }
        const std::array<bool, MAX_LIGHT>& GetShadowFlags() { return useShadow_; }
    private:
        //DirectionalLight
        DirectionalCommonLight directionalLight_{};
        std::array<PointCommonLight, kMaxPointLights> pointLights_{};
        std::array<uint32_t, MAX_LIGHT> activeCount_;
        std::array<SpotCommonLight, kMaxSpotLights> spotLights_{};
        std::array<AreaCommonLight, kMaxAreaLights> areaLights_{};
         std::array<bool, MAX_LIGHT> useShadow_;
    };

}

