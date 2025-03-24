#include "parser.h"

int main()
{
	cs2::PhysicsFile physics;
	physics.load(
		"C:\\Users\\vasie\\Desktop\\map\\world_physics.vmdl",
		"C:\\Users\\vasie\\Desktop\\map"
	);
	
	physics.displayStats();
	physics.writeTriangles(physics.getMapname() + ".tri");
	
	return 0;
}