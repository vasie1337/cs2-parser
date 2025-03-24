#pragma once
#include <cmath>
#include <iostream>
#include <map>
#include <vector>
#include <string>
#include <sstream>
#include <variant>
#include <optional>
#include <fstream>
#include <filesystem>
#include <algorithm>

namespace cs2
{
	class Vec3 {
	public:
		float x, y, z;

		Vec3() = default;
		Vec3(float x, float y, float z) : x(x), y(y), z(z) {}
	};

	class Triangle {
	public:
		Vec3 a, b, c;

		Triangle() = default;
		Triangle(Vec3 a, Vec3 b, Vec3 c) : a(a), b(b), c(c) {}
	};

	class HullFile {
	public:
		std::string name;
		std::string surface_prop;
		std::vector<Triangle> triangles;

		HullFile() = default;
		HullFile(const std::string& name, const std::string& surface_prop) : name(name), surface_prop(surface_prop) {}
	};
	
	class PhysicsFile {
	public:
		bool load(const std::string& filename, const std::string& workingDir);

		const std::vector<HullFile>& getHulls() const { return hulls; }
	private:
		std::vector<HullFile> hulls;

		void parseHull(HullFile& hull, const std::string& workingDir);

		std::vector<Vec3> parseVertices(const std::string& input);
		std::vector<int> parseIndices(const std::string& input);

		inline std::string removePath(const std::string& path) {
			auto pos = path.find_last_of("/\\");
			if (pos == std::string::npos)
				return path;
			return path.substr(pos + 1);
		}
	};
} // namespace cs2
