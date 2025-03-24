#include "../core/cs2/parser.h"
#include "renderer.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	std::vector<cs2::Triangle> triangles;
	
	std::ifstream in("de_mirage.tri", std::ios::binary);
	if (!in)
	{
		MessageBoxA(NULL, "Failed to open file: de_mirage.tri\nMake sure the file exists in the executable directory.", "Error", MB_OK | MB_ICONERROR);
		return 1;
	}

	cs2::Triangle triangle = {};
	while (in.read(reinterpret_cast<char*>(&triangle), sizeof(cs2::Triangle)))
	{
		triangles.push_back(triangle);
	}

	in.close();

	Renderer::Application app;
	if (!app.Initialize(hInstance, nCmdShow))
	{
		MessageBoxA(NULL, "Failed to initialize the application.", "Error", MB_OK | MB_ICONERROR);
		return 1;
	}

	if (!app.LoadTriangles(triangles))
	{
		MessageBoxA(NULL, "Failed to load triangles.", "Error", MB_OK | MB_ICONERROR);
		return 1;
	}

	app.Run();

	return 0;
}