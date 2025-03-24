#include "parser.h"

bool cs2::PhysicsFile::load(const std::string& filename, const std::string& workingDir)
{
	std::ifstream file(filename);
	if (!file.is_open())
	{
		std::cerr << "Failed to open file: " << filename << std::endl;
		return false;
	}

	this->filename = removePath(filename);

	std::vector<char> buffer(std::istreambuf_iterator<char>(file), {});
	buffer.push_back('\0');
	file.close();

    std::string_view data(buffer.data(), buffer.size());

    while (data.find("_class") != std::string::npos)
    {
		HullFile Hull = HullFile();

		// Filename
        size_t start = data.find("filename = \"") + 12;
        size_t end = data.find("\"", start);
        Hull.name = std::string(data.substr(start, end - start));

		// Surface Prop
        start = data.find("surface_prop = \"", end) + 16;
        end = data.find("\"", start);
        Hull.surface_prop = std::string(data.substr(start, end - start));

        hulls.push_back(Hull);

        data = data.substr(end);
    }

	this->mapname = hulls[0].name;
	this->mapname.erase(0, 5);
	this->mapname.erase(this->mapname.find("/"), this->mapname.size());

	for (auto& Hull : hulls) {
		parseHull(Hull, workingDir);
	}

	return true;
}

void cs2::PhysicsFile::writeTriangles(const std::string& filename)
{
	std::ofstream file(filename);
	if (!file.is_open())
	{
		std::cerr << "Failed to open file: " << filename << std::endl;
		return;
	}

	for (auto& Hull : hulls)
	{
		for (auto& tri : Hull.triangles)
		{
			file << reinterpret_cast<char*>(&tri) << std::endl;
		}
	}

	file.close();
}

void cs2::PhysicsFile::displayStats()
{
	std::cout << "Filename: " << filename << std::endl;
	std::cout << "Mapname: " << mapname << std::endl;

	std::unordered_map<std::string, int> surface_props;
	int total_triangles = 0;

	for (auto& Hull : hulls)
	{
		total_triangles += static_cast<int>(Hull.triangles.size());
		surface_props[Hull.surface_prop]++;
	}

	std::cout << "Total Hulls: " << hulls.size() << std::endl;
	std::cout << "Total Triangles: " << total_triangles << std::endl;

	std::cout << "Surface Props:" << std::endl;
	for (auto& [prop, count] : surface_props)
	{
		std::cout << prop << ": " << count << std::endl;
	}

	std::cout << std::endl;
}

void cs2::PhysicsFile::parseHull(HullFile& hull, const std::string& workingDir)
{
	std::string file_name = removePath(hull.name);

	std::ifstream file(workingDir + "/" + file_name);
	if (!file.is_open())
	{
		std::cerr << "Failed to open file: " << file_name << std::endl;
		return;
	}

	std::vector<char> buffer(std::istreambuf_iterator<char>(file), {});
	buffer.push_back('\0');
	file.close();

	std::string_view data(buffer.data(), buffer.size());

	size_t start = data.find("\"position$0\" \"vector3_array\"") + 31;
	size_t end = data.find("]", start);
	std::string_view vertices = data.substr(start, end - start);
	std::string vertices_str = std::string(vertices);
	vertices_str.erase(std::remove(vertices_str.begin(), vertices_str.end(), '\"'), vertices_str.end());

	start = data.find("\"position$0Indices\" \"int_array\"") + 34;
	end = data.find("]", start);
	std::string_view indices = data.substr(start, end - start);
	std::string indices_str = std::string(indices);
	indices_str.erase(std::remove(indices_str.begin(), indices_str.end(), '\"'), indices_str.end());

	std::vector<Vec3> vertex_list;
	std::vector<int> indices_list;

	vertex_list = parseVertices(vertices_str);
	indices_list = parseIndices(indices_str);

	for (size_t i = 0; i + 2 < indices_list.size(); i += 3) {
		if (indices_list[i] >= vertex_list.size() ||
			indices_list[i + 1] >= vertex_list.size() ||
			indices_list[i + 2] >= vertex_list.size()) {
			continue;
		}

		Triangle tri = Triangle();
		tri.a = vertex_list[indices_list[i]];
		tri.b = vertex_list[indices_list[i + 1]];
		tri.c = vertex_list[indices_list[i + 2]];

		hull.triangles.push_back(tri);
	}
}

std::vector<cs2::Vec3> cs2::PhysicsFile::parseVertices(const std::string& input)
{
	std::vector<cs2::Vec3> vectors;
	std::stringstream ss(input);
	std::string item;

	while (std::getline(ss, item, ',')) {
		std::stringstream vectorStream(item);
		cs2::Vec3 vec = cs2::Vec3();
		vectorStream >> vec.x >> vec.y >> vec.z;
		vectors.push_back(vec);
	}

	return vectors;
}

std::vector<int> cs2::PhysicsFile::parseIndices(const std::string& input)
{
	std::vector<int> indices;
	std::stringstream ss(input);
	std::string item;

	while (std::getline(ss, item, ',')) {
		std::stringstream indexStream(item);
		int index;
		indexStream >> index;
		indices.push_back(index);
	}

	return indices;
}
