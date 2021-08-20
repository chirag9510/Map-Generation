#include "MapGenerator.h"
#include "Core.h"
#include <string>
#include <spdlog/spdlog.h>

#include "imgui.h"
#include "imgui_sdl.h"


MapGenerator::~MapGenerator()
{
	SDL_DestroyTexture(texMap);

	WriteDatFile();
}

void MapGenerator::Generate(SDL_Renderer* mRenderer)
{
	//FastNoiseLite Init
	FastNoiseLite fn;
	fn.SetNoiseType(noiseType);
	fn.SetFractalType(fractalType);
	fn.SetSeed(iSeed);
	fn.SetFrequency(fFrequency);
	fn.SetFractalOctaves(iOctaves);
	fn.SetFractalGain(fFractalGain);
	fn.SetFractalWeightedStrength(fFractalWeightedStrength);
	fn.SetFractalLacunarity(fFractalLacunarity);

	SDL_Color color = { 255, 255, 255, 255 };
	float e = 0.0f, fx = 0.0f, fy = 0.0f;
	//for circular gradient, which allows for generation of islands
	float fXMid = static_cast<float>(Core::mWidth) / 2.0f, fYMid = static_cast<float>(Core::mHeight) / 2.0f;
	float fDistanceMiddle = glm::sqrt(fXMid * fXMid + fYMid * fYMid);			//central point of the gradient
	float fRadius = fDistanceMiddle / 6.0f;									//distnce from the center of the gradient
	
	//Create a blank surface
	SDL_Surface* surface = SDL_CreateRGBSurface(0, Core::mWidth, Core::mHeight, 32, 0, 0, 0, 0);
	SDL_SetSurfaceBlendMode(surface, SDL_BLENDMODE_BLEND);
	if (SDL_MUSTLOCK(surface))												//maybe lock it for pixel modifications
		SDL_LockSurface(surface);
	Uint32* uPixels = (Uint32*)surface->pixels;								//extract all pixels
	
	for (int y = 0; y < Core::mHeight; y++)
	{
		for (int x = 0; x < Core::mWidth; x++)
		{
			fx = static_cast<float>(x), fy = static_cast<float>(y);
			////noise range is from -1.0f to 1.0f
			e = (fn.GetNoise(fx, fy) + 0.5f * fn.GetNoise(2.0f * fx, 2.0f * fy) + 0.25f * fn.GetNoise(4.0f * fx, 4.0f * fy)) / 1.75f;
			float d = glm::sqrt((fx - fXMid) * (fx - fXMid) + (fy - fYMid) * (fy - fYMid));
			float eGradient = (d - fRadius) / fRadius;
			if (mRenderType == GRADIENT)					
				e = eGradient;									
			else if (mRenderType == ISLAND)
				e -= eGradient;							//to generate islands, subtract from gradient
			e = e > 1.0f ? 1.0f : e;					//clamp range
			e = e < -1.0f ? -1.0f : e;

			if (bEnableColors)
			{
				//set color according to biome, again the range is from -1 to 1
				if (e < fDeepWater)
					color = { static_cast<Uint8>(255.0f * fDeepWaterColor[0]), static_cast<Uint8>(255 * fDeepWaterColor[1]), static_cast<Uint8>(255 * fDeepWaterColor[2])};
				else if (e < fWater)
					color = { static_cast<Uint8>(255.0f * fWaterColor[0]), static_cast<Uint8>(255 * fWaterColor[1]), static_cast<Uint8>(255 * fWaterColor[2]) };
				else if (e < fBeach)
					color = { static_cast<Uint8>(255.0f * fBeachColor[0]), static_cast<Uint8>(255 * fBeachColor[1]), static_cast<Uint8>(255 * fBeachColor[2]) };
				else if (e < fGrassland)
					color = { static_cast<Uint8>(255.0f * fGrassColor[0]), static_cast<Uint8>(255 * fGrassColor[1]), static_cast<Uint8>(255 * fGrassColor[2])};
				else if (e < fForest)
					color = { static_cast<Uint8>(255.0f * fForestColor[0]), static_cast<Uint8>(255 * fForestColor[1]), static_cast<Uint8>(255 * fForestColor[2]) };
				else if (e < fRock)
					color = { static_cast<Uint8>(255.0f * fRockColor[0]), static_cast<Uint8>(255 * fRockColor[1]), static_cast<Uint8>(255 * fRockColor[2]) };
				else
					color = { static_cast<Uint8>(255.0f * fSnowColor[0]), static_cast<Uint8>(255 * fSnowColor[1]), static_cast<Uint8>(255 * fSnowColor[2])};
			}
			else
				color.r = color.g = color.b = static_cast<Uint8>(117.0f * (1 + e));				//since e can be -1 or 1, so the colors should be 0 - 255

			//finally set the pixel color
			uPixels[(y * surface->w) + x] = SDL_MapRGBA(surface->format, color.r, color.g, color.b, 255);
		}
	}
	if (SDL_MUSTLOCK(surface))					
		SDL_UnlockSurface(surface);

	//create texture from the surface
	if(texMap)	
		SDL_DestroyTexture(texMap);
	texMap = SDL_CreateTextureFromSurface(mRenderer, surface);
	rectTexMap = { 0, 0, surface->w, surface->h };
	SDL_FreeSurface(surface);
}

void MapGenerator::Render(SDL_Renderer* mRenderer)
{
	SDL_RenderCopy(mRenderer, texMap, 0, &rectTexMap);

	//ImGui interface 
	ImGui::NewFrame();
	if (ImGui::Begin("Noise Configuration"))
	{
		if (ImGui::Button("Generate!"))
			Generate(mRenderer);
		ImGui::Text("or Press 'Space'");
		ImGui::NewLine();

		if (bEnableColors)
		{
			if (ImGui::Button("Colored : On"))
				bEnableColors = false;
		}
		else
			if (ImGui::Button("Colors : Off"))
				bEnableColors = true;

		int iSelection = mRenderType;
		const char* szRenderType[] = { "Default", "Island","Gradient"};
		ImGui::ListBox("Render Type", &iSelection, szRenderType, ARRAYSIZE(szRenderType));
		mRenderType = static_cast<RenderType>(iSelection);

		iSelection = noiseType;
		const char* szNoiseType[] = { "OpenSimplex2", "OpenSimplex2S","Cellular","Perlin","ValueCubic","Value" };
		ImGui::ListBox("Noise Type", &iSelection, szNoiseType, ARRAYSIZE(szNoiseType));
		noiseType = static_cast<FastNoiseLite::NoiseType>(iSelection);

		iSelection = fractalType;
		const char* szFractalType[] = { "None", "FBm","Ridged","PingPong","DomainWarpProgressive","DomainWarpIndependent"};
		ImGui::ListBox("Fractal Type", &iSelection, szFractalType, ARRAYSIZE(szFractalType));
		fractalType = static_cast<FastNoiseLite::FractalType>(iSelection);

		ImGui::InputInt("Seed", &iSeed);
		ImGui::InputFloat("Frequency", &fFrequency);
		ImGui::InputInt("Octaves", &iOctaves);
		ImGui::InputFloat("Fractal Gain", &fFractalGain);
		ImGui::InputFloat("Fractal Weighted Strength", &fFractalWeightedStrength);
		ImGui::InputFloat("Fractal Lacunarity", &fFractalLacunarity);

		ImGui::Text("Biome Colors");
		ImGui::ColorEdit3("DeepWater color", fDeepWaterColor);
		ImGui::ColorEdit3("Water color", fWaterColor);
		ImGui::ColorEdit3("Beach color", fBeachColor);
		ImGui::ColorEdit3("Grass color", fGrassColor);
		ImGui::ColorEdit3("Forest color", fForestColor);
		ImGui::ColorEdit3("Rock color", fRockColor);
		ImGui::ColorEdit3("Snow color", fSnowColor);

		ImGui::Text("Biome Range");
		ImGui::SliderFloat("DeepWater", &fDeepWater,-1.0f, 1.0f);
		ImGui::SliderFloat("Water", &fWater,-1.0f, 1.0f);
		ImGui::SliderFloat("Beach", &fBeach,-1.0f, 1.0f);
		ImGui::SliderFloat("Grass", &fGrassland,-1.0f, 1.0f);
		ImGui::SliderFloat("Forest", &fForest,-1.0f, 1.0f);
		ImGui::SliderFloat("Rock", &fRock,-1.0f, 1.0f);
		ImGui::SliderFloat("Snow", &fSnow,-1.0f, 1.0f);

		ImGui::NewLine();
		if (ImGui::Button("Reset to Defaults"))
			ResetToDefaults();
	}
	ImGui::End();

	ImGui::Render();
	ImGuiSDL::Render(ImGui::GetDrawData());
}


void MapGenerator::ResetToDefaults()
{
	noiseType = FastNoiseLite::NoiseType_OpenSimplex2;
	fractalType = FastNoiseLite::FractalType_FBm;
	iSeed = 428, iOctaves = 3;
	fFrequency = 0.01f;
	fFractalGain = .25f;
	fFractalWeightedStrength = -1.4f,
		fFractalLacunarity = 3.5f;

	mRenderType = DEFAULT;
	bEnableColors = false;

	fDeepWaterColor[0] = 0.2, fDeepWaterColor[1] = 0.458, fDeepWaterColor[2] = 0.517, fDeepWater = -0.4;
	fWaterColor[0] = 0.462, fWaterColor[1] = 0.713, fWaterColor[2] = 0.717, fWater = -0.300;
	fBeachColor[0] = 0.992, fBeachColor[1] = 0.847, fBeachColor[2] = 0.709, fBeach = -0.200;
	fGrassColor[0] = 0.870, fGrassColor[1] = 0.898, fGrassColor[2] = 0.596, fGrassland = 0.000;
	fForestColor[0] = 0.250, fForestColor[1] = 0.466, fForestColor[2] = 0.000, fForest = 0.550;
	fRockColor[0] = 0.502, fRockColor[1] = 0.490, fRockColor[2] = 0.490, fRock = 0.750;
	fSnowColor[0] = 0.941, fSnowColor[1] = 0.941, fSnowColor[2] = 0.941, fSnow = 1.000;

}

void MapGenerator::ReadDatFile()
{
	FILE* fileDat;
	if (fopen_s(&fileDat, "./config.dat", "r"))
	{
		//if the config file isnt found, then set all vars to their default values a config.dat will be generated later by WriteDatFile()
		spdlog::error("config.dat not found..restoring to default values");
		ResetToDefaults();

	}
	else
	{
		int iType;
		fscanf_s(fileDat, "%d", &iType);
		noiseType = static_cast<FastNoiseLite::NoiseType>(iType);
		noiseType = noiseType > FastNoiseLite::NoiseType_Value ? FastNoiseLite::NoiseType_Value : noiseType;
		fscanf_s(fileDat, "%d", &iType);
		fractalType = static_cast<FastNoiseLite::FractalType>(iType);
		fractalType = fractalType > FastNoiseLite::FractalType_DomainWarpIndependent ? FastNoiseLite::FractalType_DomainWarpIndependent : fractalType;

		fscanf_s(fileDat, "%d", &iSeed);
		fscanf_s(fileDat, "%f", &fFrequency);
		fscanf_s(fileDat, "%d", &iOctaves);
		fscanf_s(fileDat, "%f", &fFractalGain);
		fscanf_s(fileDat, "%f", &fFractalWeightedStrength);
		fscanf_s(fileDat, "%f", &fFractalLacunarity);

		fscanf_s(fileDat, "%d", &iType);
		bEnableColors = iType;

		fscanf_s(fileDat, "%d", &iType);
		mRenderType = static_cast<RenderType>(iType);
		mRenderType = mRenderType > GRADIENT ? GRADIENT : mRenderType;

		fscanf_s(fileDat, "%f,%f,%f,%f", &fDeepWaterColor[0], &fDeepWaterColor[1], &fDeepWaterColor[2], &fDeepWater);
		fscanf_s(fileDat, "%f,%f,%f,%f", &fWaterColor[0], &fWaterColor[1], &fWaterColor[2], &fWater);
		fscanf_s(fileDat, "%f,%f,%f,%f", &fBeachColor[0], &fBeachColor[1], &fBeachColor[2], &fBeach);
		fscanf_s(fileDat, "%f,%f,%f,%f", &fGrassColor[0], &fGrassColor[1], &fGrassColor[2], &fGrassland);
		fscanf_s(fileDat, "%f,%f,%f,%f", &fForestColor[0], &fForestColor[1], &fForestColor[2], &fForest);
		fscanf_s(fileDat, "%f,%f,%f,%f", &fRockColor[0], &fRockColor[1], &fRockColor[2], &fRock);
		fscanf_s(fileDat, "%f,%f,%f,%f", &fSnowColor[0], &fSnowColor[1], &fSnowColor[2], &fSnow);

		fclose(fileDat);
	}
}

void MapGenerator::WriteDatFile()
{
	FILE* fileDat;
	fopen_s(&fileDat, "./config.dat", "w+");

	fprintf_s(fileDat, "%d\n", noiseType);
	fprintf_s(fileDat, "%d\n", fractalType);

	fprintf_s(fileDat, "%d\n", iSeed);
	fprintf_s(fileDat, "%f\n", fFrequency);
	fprintf_s(fileDat, "%d\n", iOctaves);
	fprintf_s(fileDat, "%f\n", fFractalGain);
	fprintf_s(fileDat, "%f\n", fFractalWeightedStrength);
	fprintf_s(fileDat, "%f\n", fFractalLacunarity);

	fprintf_s(fileDat, "%d\n", bEnableColors);
	fprintf_s(fileDat, "%d\n", mRenderType);

	fprintf_s(fileDat, "%f,%f,%f,%f\n", fDeepWaterColor[0], fDeepWaterColor[1], fDeepWaterColor[2], fDeepWater);
	fprintf_s(fileDat, "%f,%f,%f,%f\n", fWaterColor[0], fWaterColor[1], fWaterColor[2], fWater);
	fprintf_s(fileDat, "%f,%f,%f,%f\n", fBeachColor[0], fBeachColor[1], fBeachColor[2], fBeach);
	fprintf_s(fileDat, "%f,%f,%f,%f\n", fGrassColor[0], fGrassColor[1], fGrassColor[2], fGrassland);
	fprintf_s(fileDat, "%f,%f,%f,%f\n", fForestColor[0], fForestColor[1], fForestColor[2], fForest);
	fprintf_s(fileDat, "%f,%f,%f,%f\n", fRockColor[0], fRockColor[1], fRockColor[2], fRock);
	fprintf_s(fileDat, "%f,%f,%f,%f\n", fSnowColor[0], fSnowColor[1], fSnowColor[2], fSnow);

	fclose(fileDat);
}