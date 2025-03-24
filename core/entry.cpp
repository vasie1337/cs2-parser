#include "parser.h"

int main()
{
	cs2::PhysicsFile physics;
	physics.load(
		"C:\\Users\\vasie\\Desktop\\map\\world_physics.vmdl",
		"C:\\Users\\vasie\\Desktop\\map"
	);

	auto hulls = physics.getHulls();

	for (auto& hull : hulls)
	{
		std::cout << "Hull: " << hull.name << std::endl;
		std::cout << "Surface Prop: " << hull.surface_prop << std::endl;
		std::cout << "Triangles: " << hull.triangles.size() << std::endl;
	}

	return 0;
}