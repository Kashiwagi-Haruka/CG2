#include "SavePoint.h"

SavePoint::~SavePoint()
{}

void SavePoint::Initialize()
{
	savePoint_ = std::make_unique<Primitive>();
	savePoint_->Initialize(Primitive::PrimitiveName::Sphere);
}

void SavePoint::Update()
{
	savePoint_->Update(); }

void SavePoint::Draw()
{
	savePoint_->Draw(); }
