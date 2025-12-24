#include "Camera.h"
#include "Function.h"

Camera::Camera(): transform_({{1.0f, 1.0f, 1.0f},{0.0f, 0.0f, 0.0f},{0.0f, 0.0f, 0.0f}})
    ,fovY(0.45f)
    , aspectRatio(float(WinApp::kClientWidth) / float(WinApp::kClientHeight))
	, nearZ(0.1f)
	, farZ(100.0f),
      worldMatrix_(Function::MakeAffineMatrix({1.0f, 1.0f, 1.0f}, transform_.rotate, transform_.translate))
	, viewMatrix_(Function::Inverse(worldMatrix_)),
      projectionMatrix_(Function::MakePerspectiveFovMatrix(fovY, aspectRatio, nearZ, farZ)), viewProjectionMatrix_(Function::Multiply(viewMatrix_, projectionMatrix_)) 
{}


void Camera::Update(){

	worldMatrix_ = Function::MakeAffineMatrix({1.0f,1.0f,1.0f}, transform_.rotate, transform_.translate);
	viewMatrix_ = Function::Inverse(worldMatrix_);
	projectionMatrix_ = Function::MakePerspectiveFovMatrix(fovY, aspectRatio, nearZ, farZ);
	viewProjectionMatrix_ = Function::Multiply(viewMatrix_, projectionMatrix_);

}