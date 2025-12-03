#pragma once
class BaseScene {

	protected:
	bool isSceneEnd = false;

	public: 

	virtual ~BaseScene() = default;
	virtual void Initialize();
	virtual void Update();
	virtual void Draw();
	virtual void Finalize();
};
