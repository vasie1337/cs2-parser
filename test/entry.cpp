#include "../core/cs2/parser.h"

int main()
{
	std::vector<cs2::Triangle> triangles;
	
	std::ifstream in("de_dust2.tri", std::ios::binary);
	if (!in)
	{
		std::cerr << "Failed to open file: dust.tri" << std::endl;
		return 1;
	}

	cs2::Triangle triangle = {};
	while (in.read(reinterpret_cast<char*>(&triangle), sizeof(cs2::Triangle)))
	{
		triangles.push_back(triangle);
	}

	std::cout << "Loaded " << triangles.size() << " triangles." << std::endl;


	return 0;
}