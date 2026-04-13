#pragma once

#include "Engine/Editor/EditorTool/Inspector/Inspector.h"
#include "Transform.h"

class Primitive;

class EditorPrimitive {
public:
	static InspectorMaterial CaptureMaterial(const Primitive* primitive);
	static void ApplyEditorValues(Primitive* primitive, const Transform& transform, const InspectorMaterial& material);
};