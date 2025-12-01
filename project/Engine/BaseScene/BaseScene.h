#pragma once
class BaseScene {

	public: 

	virtual ~BaseScene() = default;
	virtual void Initialize();
	virtual void Update();
	virtual void Draw();
	virtual void Finalize();
};
