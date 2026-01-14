#include "Primitive.h"
#include "Object3d/Object3dCommon.h"
void Primitive::Initialize(PrimitiveName name) {

	primitiveName_ = name;

	switch (primitiveName_) {
	case Primitive::Plane:
		break;
	case Primitive::Circle:
		break;
	case Primitive::Ring:
		break;
	case Primitive::Sphere:
		break;
	case Primitive::Torus:
		break;
	case Primitive::Cylinder:
		break;
	case Primitive::Cone:
		break;
	case Primitive::Triangle:
		break;
	case Primitive::Box:
		break;
	default:
		break;
	}

	camera_ = Object3dCommon::GetInstance()->GetDefaultCamera();


}
void Primitive::Update() {

}
void Primitive::Draw() {


}