#pragma once
class Camera;
class CameraController {


	Camera* camera_ = nullptr;


	public:
	~CameraController();
	void Initialize();
	void Update();
	void SetCamera(Camera* camera);
	Camera* GetCamera();

};
