#include "LightManager.h"
#include"Object3d/Object3dCommon.h"
#include"imgui.h"


void Yoshida::LightManager::SetActiveLightCount(Lights lightType, uint32_t count)
{
    activeCount_[lightType] = count;
}

void Yoshida::LightManager::SetPointLight(const PointCommonLight& pointLight, uint32_t index)
{
    assert(index < activeCount_[POINT]);
    pointLights_[index] = pointLight;
}

void Yoshida::LightManager::SetAreaLight(const AreaCommonLight& areaLight, uint32_t index)
{
    assert(index < activeCount_[AREA]);
    areaLights_[index] = areaLight;
}

void Yoshida::LightManager::SetSpotLight(const SpotCommonLight& spotLight, uint32_t index)
{
    assert(index < activeCount_[SPOT]);
    spotLights_[index] = spotLight;
}

void Yoshida::LightManager::Initialize()
{
    directionalLight_.color = { 1.0f, 1.0f, 0.75f, 1.0f };
    directionalLight_.direction = { 0.0f, 1.0f, 0.0f };
    directionalLight_.intensity = 0.25f;

    for (int i = 0; i < MAX_LIGHT; ++i) {
        useShadow_[i] = false;
    }

}

void Yoshida::LightManager::Update()
{
    Object3dCommon::GetInstance()->SetDirectionalLight(directionalLight_);
    Object3dCommon::GetInstance()->SetPointLights(pointLights_.data(), activeCount_[POINT]);
    Object3dCommon::GetInstance()->SetSpotLights(spotLights_.data(), activeCount_[SPOT]);
    Object3dCommon::GetInstance()->SetAreaLights(areaLights_.data(), activeCount_[AREA]);
    Object3dCommon::GetInstance()->SetShadowMapEnabled(useShadow_[DIRECTIONAL], useShadow_[POINT], useShadow_[SPOT], useShadow_[AREA]);


#ifdef USE_IMGUI
    if (ImGui::TreeNode("Light")) {

        ImGui::DragFloat3("Area0Position", &areaLights_.at(0).position.x, 0.1f);
        ImGui::DragFloat3("Area1Position", &areaLights_.at(1).position.x, 0.1f);

        ImGui::Checkbox("DirectionalShadow", &useShadow_[DIRECTIONAL]);
        ImGui::Checkbox("PointShadow", &useShadow_[POINT]);
        ImGui::Checkbox("SpotShadow", &useShadow_[SPOT]);
        ImGui::Checkbox("AreaShadow", &useShadow_[AREA]);
        pointLights_.at(0).shadowEnabled = useShadow_[POINT] ? 1 : 0;
        spotLights_.at(0).shadowEnabled = useShadow_[SPOT] ? 1 : 0;
        areaLights_.at(0).shadowEnabled = useShadow_[AREA] ? 1 : 0;
        ImGui::TreePop();
    }
#endif
}

void Yoshida::LightManager::ClearLights()
{
    SetActiveLightCount(POINT, 0);
    SetActiveLightCount(SPOT, 0);
    SetActiveLightCount(AREA, 0);

    for (auto spotLight : spotLights_) {
        spotLight = {};
    }

    for (auto areaLight : areaLights_) {
        areaLight = {};
    }

    for (auto pointLight : pointLights_) {
        pointLight = {};
    }
}
