#pragma once
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

	void Initialize(PrimitiveName name);


};
