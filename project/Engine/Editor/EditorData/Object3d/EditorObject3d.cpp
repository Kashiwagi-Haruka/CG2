#include "EditorObject3d.h"

#include "Object3d/Object3d.h"

InspectorMaterial EditorObject3d::CaptureMaterial(const Object3d* object) {
	if (!object) {
		return {};
	}
	return {
	    object->GetColor(),           object->IsLightingEnabled(), object->GetShininess(),          object->GetEnvironmentCoefficient(),
	    object->IsGrayscaleEnabled(), object->IsSepiaEnabled(),    object->GetDistortionStrength(), object->GetDistortionFalloff(),
	};
}

void EditorObject3d::ApplyEditorValues(Object3d* object, const Transform& transform, const InspectorMaterial& material) {
	if (!object) {
		return;
	}
	object->SetTransform(transform);
	object->SetColor(material.color);
	object->SetEnableLighting(material.enableLighting);
	object->SetShininess(material.shininess);
	object->SetEnvironmentCoefficient(material.environmentCoefficient);
	object->SetGrayscaleEnabled(material.grayscaleEnabled);
	object->SetSepiaEnabled(material.sepiaEnabled);
	object->SetDistortionStrength(material.distortionStrength);
	object->SetDistortionFalloff(material.distortionFalloff);
	object->SetUvTransform(material.uvScale, material.uvRotate, material.uvTranslate, material.uvAnchor);
}