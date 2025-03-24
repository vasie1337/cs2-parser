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
#include <unordered_map>

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
		/// <summary>
		/// Load a physics file from a given filename and working directory.
		/// </summary>
		/// <param name="filename">
		/// The filename of the physics file.
		/// </param>
		/// <param name="workingDir">
		/// The working directory of the physics file.
		/// </param>
		/// <returns>
		/// Returns true if the file was loaded successfully, false otherwise.
		/// </returns>
		bool load(const std::string& filename, const std::string& workingDir);

		/// <summary>
		/// Write the triangles of the physics file to a given filename.
		/// </summary>
		/// <param name="filename">
		/// The filename to write the triangles to.
		/// </param>
		void writeTriangles(const std::string& filename);

		/// <summary>
		/// Display the statistics of the physics file.
		/// </summary>
		void displayStats();

		/// <summary>
		/// Get the hulls of the physics file.
		/// </summary>
		/// <returns>
		/// Returns the hulls of the physics file.
		/// </returns>
		const std::vector<HullFile>& getHulls() const { return hulls; }

		/// <summary>
		/// Get the filename of the physics file.
		/// </summary>
		/// <returns>
		/// Returns the filename of the physics file.
		/// </returns>
		const std::string& getFilename() const { return filename; }

		/// <summary>
		/// Get the map name of the physics file.
		/// </summary>
		/// <returns>
		/// Returns the map name of the physics file.
		/// </returns>
		const std::string& getMapname() const { return mapname; }

	private:
		std::string filename;
		std::string mapname;

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
