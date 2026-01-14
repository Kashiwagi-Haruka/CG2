#pragma once
#include <cstdint>
class Primitive {

	private:


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

	
	public:

	void Initialize(PrimitiveName name);
	void Update();
	void Draw();
};
