#pragma once
#include <SDL.h>
#include <glm.hpp>
#include <vector>
#include <memory>
#include "FastNoiseLite.h"


class MapGenerator
{
	enum RenderType { DEFAULT, ISLAND, GRADIENT };

	SDL_Texture* texMap;
	SDL_Rect rectTexMap;
	float fDeepWater, fWater, fBeach, fGrassland, fForest, fRock, fSnow;
	//since imgui uses float arrays for ColorEdit
	float fDeepWaterColor[3], fGrassColor[3], fBeachColor[3], fWaterColor[3], fForestColor[3], fRockColor[3], fSnowColor[3];

	FastNoiseLite::NoiseType noiseType;
	FastNoiseLite::FractalType fractalType;
	int iSeed, iOctaves;
	float fFrequency, fFractalGain ,fFractalWeightedStrength ,fFractalLacunarity;
	RenderType mRenderType;
	bool bEnableColors;

	void ResetToDefaults();

public:

	MapGenerator() = default;
	~MapGenerator();

	void Generate(SDL_Renderer* mRenderer);
	void Render(SDL_Renderer* mRenderer);
	void ReadDatFile();
	void WriteDatFile();
};
