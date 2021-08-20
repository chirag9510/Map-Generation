#pragma once
#include <SDL.h>
#include <SDL_ttf.h>
#include <cstdint>
#include <memory>
#include "MapGenerator.h"

class MapGenerator;

class Core
{
	SDL_Window* mWindow;
	SDL_Renderer* mRenderer;

	int iTicksLastFrame;
	bool bRunning;

	std::unique_ptr<MapGenerator> mapGenerator;

	void ProcessInput();
	void Update();
	void Render();

public:
	static int mWidth, mHeight;

	Core();
	~Core() = default;

	bool Init();
	void Run();
	void Destroy();

};

