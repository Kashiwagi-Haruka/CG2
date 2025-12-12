#include "DirectionalLight.h"
#include <assert.h>
#include "Object3dCommon.h"
void DirectionalLightHelper::Initialize(){

		// Lightバッファ
	directionalLightResource_ = obj3dCommon_->CreateBufferResource(sizeof(DirectionalLight));
	assert(directionalLightResource_);
	directionalLightData_ = nullptr;
	directionalLightResource_->Map(0, nullptr, reinterpret_cast<void**>(&directionalLightData_));
	assert(directionalLightData_);
	*directionalLightData_ = {
	    {1.0f, 1.0f, 1.0f, 1.0f},
        {0.0f, -1.0f, 0.0f},
        1.0f
    };
	directionalLightResource_->Unmap(0, nullptr);

}

void DirectionalLightHelper::Update() {}