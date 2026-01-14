#pragma once
#include <cstdint>

class Camera;

class Primitive {

	public:
	enum PrimitiveName {
		Plane,
		Circle,
		Ring,
		Sphere,
		Torus,
		Cylinder,
		Cone,
		Triangle,
		Box,
	};

	private:

		PrimitiveName primitiveName_;
	    Camera* camera_;

	
	public:

	void Initialize(PrimitiveName name);
	void Update();
	void Draw();
};
