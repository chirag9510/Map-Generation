#include "Core.h"
#include <SDL_image.h>
#include <string>
#include <spdlog/spdlog.h>

#include "imgui.h"
#include "imgui_sdl.h"
#include "imgui_impl_sdl.h"

#define FPS_CAP 1000 / 60

int Core::mWidth = 800;
int Core::mHeight = 600;

Core::Core()
{
	iTicksLastFrame = 0;
	bRunning = true;
	mapGenerator = std::make_unique<MapGenerator>();
}

bool Core::Init()
{
	//SDL Init
	SDL_Init(SDL_INIT_VIDEO);
	mWindow = SDL_CreateWindow("Map", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, mWidth, mHeight, SDL_WINDOW_MAXIMIZED | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
	if (!mWindow)
	{
		spdlog::error("mWindow : " + std::string(SDL_GetError()));
		return false;
	}
	SDL_GetWindowSize(mWindow, &mWidth, &mHeight);
	mRenderer = SDL_CreateRenderer(mWindow, -1, SDL_RENDERER_ACCELERATED);
	if (!mRenderer)
	{
		spdlog::error("mRenderer : " + std::string(SDL_GetError()));
		return false;
	}
	SDL_SetRenderDrawColor(mRenderer, 20, 20, 20, 255);
	SDL_SetRenderDrawBlendMode(mRenderer, SDL_BLENDMODE_BLEND);
	IMG_Init(IMG_INIT_PNG);
	TTF_Init();

	//ImGui SDL Init
	ImGui::CreateContext();
	ImGuiSDL::Initialize(mRenderer, mWidth, mHeight);
	
	//Mapgenerator Init
	mapGenerator->ReadDatFile();
	mapGenerator->Generate(mRenderer);

	return true;
}

void Core::Run()
{
	while (bRunning)
	{
		ProcessInput();
		Update();
		Render();
	}
}


void Core::ProcessInput()
{
	SDL_Event e;
	while (SDL_PollEvent(&e))
	{
		// ImGui SDL input
		ImGui_ImplSDL2_ProcessEvent(&e);
		ImGuiIO& io = ImGui::GetIO();
		int mouseX, mouseY;
		const int buttons = SDL_GetMouseState(&mouseX, &mouseY);
		io.MousePos = ImVec2(mouseX, mouseY);
		io.MouseDown[0] = buttons & SDL_BUTTON(SDL_BUTTON_LEFT);

		switch (e.type)
		{
		case SDL_KEYDOWN:
			switch (e.key.keysym.sym)
			{
			case SDLK_SPACE:
				//can also generate by pressing space
				mapGenerator->Generate(mRenderer);
				break;
			case SDLK_ESCAPE:
				bRunning = false;
				break;
			}
		
			break;

		case SDL_KEYUP:

			break;
		
		case SDL_WINDOWEVENT:
			if (e.window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
			{
				SDL_GetWindowSize(mWindow, &mWidth, &mHeight);
				//resize for imgui
				io.DisplaySize.x = static_cast<float>(e.window.data1);
				io.DisplaySize.y = static_cast<float>(e.window.data2);
			}	
			break;

		case SDL_QUIT:
			bRunning = false;
			break;
		}

	}
}

void Core::Update()
{
	//frame cap
	//modify FPS_CAP to set framerate
	int iFrameLimit = FPS_CAP - (SDL_GetTicks() - iTicksLastFrame);
	if (iFrameLimit <= FPS_CAP && iFrameLimit >= 0)
		SDL_Delay(iFrameLimit);

	iTicksLastFrame = SDL_GetTicks();

}

void Core::Render()
{
	SDL_RenderClear(mRenderer);

	mapGenerator->Render(mRenderer);

	SDL_RenderPresent(mRenderer);
}

void Core::Destroy()
{
	SDL_DestroyRenderer(mRenderer); 
	SDL_DestroyWindow(mWindow);

	ImGuiSDL::Deinitialize();
	ImGui::DestroyContext();

	TTF_Quit();
	IMG_Quit();
	SDL_Quit();
}