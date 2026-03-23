#include "Coffee.h"
#include "Model/ModelManager.h"
Coffee::Coffee() {
	coffeeObj_ = std::make_unique<Object3d>(); }

void Coffee::Initialize() { coffeeObj_->Initialize();
	ModelManager::GetInstance()->LoadGltfModel("Resources/TD3_4month/3d/Coffee", "Coffee");
	coffeeObj_->SetModel("Resources/TD3_4month/3d/Coffee,Coffee");
	}

void Coffee::Update() {

coffeeObj_->Update(); }

void Coffee::Draw() {
	coffeeObj_->Draw(); }
