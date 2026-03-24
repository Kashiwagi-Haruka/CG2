#include "EditorPrimitive.h"

#include "Primitive/Primitive.h"

InspectorMaterial EditorPrimitive::CaptureMaterial(const Primitive* primitive) {
	if (!primitive) {
		return {};
	}
	return {
	    primitive->GetColor(),           primitive->IsLightingEnabled(), primitive->GetShininess(),          primitive->GetEnvironmentCoefficient(),
	    primitive->IsGrayscaleEnabled(), primitive->IsSepiaEnabled(),    primitive->GetDistortionStrength(), primitive->GetDistortionFalloff(),
	};
}

void EditorPrimitive::ApplyEditorValues(Primitive* primitive, const Transform& transform, const InspectorMaterial& material) {
	if (!primitive) {
		return;
	}
	primitive->SetTransform(transform);
	primitive->SetColor(material.color);
	primitive->SetEnableLighting(material.enableLighting);
	primitive->SetShininess(material.shininess);
	primitive->SetEnvironmentCoefficient(material.environmentCoefficient);
	primitive->SetGrayscaleEnabled(material.grayscaleEnabled);
	primitive->SetSepiaEnabled(material.sepiaEnabled);
	primitive->SetDistortionStrength(material.distortionStrength);
	primitive->SetDistortionFalloff(material.distortionFalloff);
	primitive->SetUvTransform(material.uvScale, material.uvRotate, material.uvTranslate, material.uvAnchor);
}