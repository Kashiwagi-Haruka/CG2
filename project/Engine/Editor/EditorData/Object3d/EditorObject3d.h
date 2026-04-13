#pragma once

#include "Engine/Editor/Inspector/Inspector.h"
#include "Transform.h"

class Object3d;

class EditorObject3d {
public:
	static InspectorMaterial CaptureMaterial(const Object3d* object);
	static void ApplyEditorValues(Object3d* object, const Transform& transform, const InspectorMaterial& material);
};