#pragma once
#include <iostream>
#include <vector>
#include <string>

namespace cs2
{
	class Vec3 {
	public:
		float x, y, z;
		Vec3(float x, float y, float z) : x(x), y(y), z(z) {}
	};

	class Triangle {
	public:
		Vec3 a, b, c;
		Triangle(Vec3 a, Vec3 b, Vec3 c) : a(a), b(b), c(c) {}
	};

	class HullFile {
	public:
		std::string name;
		std::string surface_prop;
		std::vector<Triangle> triangles;
		HullFile(const std::string& name, const std::string& surface_prop) : name(name), surface_prop(surface_prop) {}
	};
	
	class PhysicsFile {
	public:
		void load(const std::string& filename);

		const std::vector<HullFile>& getHulls() const { return hulls; }
	private:
		std::vector<HullFile> hulls;

		void parseHull(const std::string& line);

		std::vector<int> parseIndices(const std::string& line);
		std::vector<float> parseVertices(const std::string& line);
	};
} // namespace cs2
