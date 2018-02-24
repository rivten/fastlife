#include <stdio.h>
#include <rivten.h>
#include <rivten_math.h>
#include <random.h>

#ifdef _WIN32
#include <SDL.h>
#else
#include <SDL2/SDL.h>
#endif

typedef bool b8;

global_variable b8 GlobalRunning = true;
global_variable u32 GlobalWindowWidth = 1024 + 128 + 64 + 32 + 16 + 8;
global_variable u32 GlobalWindowHeight = 512 + 128 + 64;
global_variable u32 TileSize = 2;
global_variable u32 UniverseWidth = GlobalWindowWidth / TileSize;
global_variable u32 UniverseHeight = GlobalWindowHeight / TileSize;

#define GetUniverseTile(Universe, X, Y) (Universe)[(X) + (Y) * UniverseWidth]

inline u8
GetNeighborCount(u8* Universe, u32 Index)
{
	u8 Count = 0;
	u32 IndexX = Index % UniverseWidth;
	u32 IndexY = Index / UniverseWidth;
	for(s32 Y = -1; Y <= 1; ++Y)
	{
		for(s32 X = -1; X <= 1; ++X)
		{
			if(Y != 0 || X != 0)
			{
				if(GetUniverseTile(Universe,
							(IndexX + X) % UniverseWidth,
							(IndexY + Y) % UniverseHeight)
						== 1)
				{
					++Count;
				}
			}
		}
	}
	return(Count);
}

int main(int ArgumentCount, char** Arguments)
{
	s32 SDLInitResult = SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER | SDL_INIT_HAPTIC | SDL_INIT_AUDIO);
	Assert(SDLInitResult == 0);

	SDL_Window* Window = SDL_CreateWindow("Fast Life", 
			SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
			GlobalWindowWidth, GlobalWindowHeight, 0);
	Assert(Window);

	u32 RendererFlags = SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC;
	SDL_Renderer* Renderer = SDL_CreateRenderer(Window, -1, RendererFlags);
	Assert(Renderer);

	// NOTE(hugo) : Timing info
	u32 LastCounter = SDL_GetTicks();
	u32 MonitorRefreshHz = 60;
	SDL_DisplayMode DisplayMode = {};
	if(SDL_GetCurrentDisplayMode(0, &DisplayMode) == 0)
	{
		if(DisplayMode.refresh_rate != 0)
		{
			MonitorRefreshHz = DisplayMode.refresh_rate;
		}
	}
	float GameUpdateHz = (MonitorRefreshHz / 2.0f);
	u32 TargetMSPerFrame = (u32)(1000.0f / GameUpdateHz);

	random_series Entropy = RandomSeed(0);

	u32 SquareCount = UniverseWidth * UniverseHeight;
	u8* Universes = AllocateArray(u8, 2 * SquareCount);
	u8* CurrentUniverse = Universes;
	u8* NextUniverse = Universes + SquareCount;

	// NOTE(hugo): Universe random init
	for(u32 SquareIndex = 0; SquareIndex < SquareCount; ++SquareIndex)
	{
#if 1
		u32 Random = RandomNextU32(&Entropy);
		CurrentUniverse[SquareIndex] = (Random & 0x00000001 != 0);
#else
		if(SquareIndex == 1000 ||
				SquareIndex == 1001 ||
				SquareIndex == 1002)
		{
			CurrentUniverse[SquareIndex] = 1;
		}
		else
		{
			CurrentUniverse[SquareIndex] = 0;
		}
#endif
	}

	while(GlobalRunning)
	{
		SDL_Event Event = {};
		while(SDL_PollEvent(&Event))
		{
			switch(Event.type)
			{
				case SDL_QUIT:
					{
						GlobalRunning = false;
					} break;
				default:
					{
					} break;
			}
		}

		// NOTE(hugo): Update
		// {
		for(u32 SquareIndex = 0; SquareIndex < SquareCount; ++SquareIndex)
		{
			NextUniverse[SquareIndex] = CurrentUniverse[SquareIndex];

			u8 NeighborCount = GetNeighborCount(CurrentUniverse, SquareIndex);
			if(CurrentUniverse[SquareIndex] == 0)
			{
				if(NeighborCount == 3)
				{
					NextUniverse[SquareIndex] = 1;
				}
			}
			else if(CurrentUniverse[SquareIndex] == 1 &&
					(NeighborCount < 2 || NeighborCount > 3))
			{
				NextUniverse[SquareIndex] = 0;
			}
		}
		// }

		// NOTE(hugo): Switch Universe Tables
		// {
		{
			u8* TempUniverse = CurrentUniverse;
			CurrentUniverse = NextUniverse;
			NextUniverse = TempUniverse;
		}
		// }

		// NOTE(hugo): Render
		// {
		SDL_SetRenderDrawColor(Renderer, 255, 255, 255, 255);
		SDL_RenderClear(Renderer);

		SDL_SetRenderDrawColor(Renderer, 0, 255, 0, 255);
		for(u32 SquareIndex = 0; SquareIndex < SquareCount; ++SquareIndex)
		{
			if(CurrentUniverse[SquareIndex] == 1)
			{
#if 0
				SDL_RenderDrawPoint(Renderer,
						SquareIndex % GlobalWindowWidth,
						SquareIndex / GlobalWindowWidth);
#else
				SDL_Rect TileRect = {};
				TileRect.x = (SquareIndex % UniverseWidth) * TileSize;
				TileRect.y = (SquareIndex / UniverseWidth) * TileSize;
				TileRect.w = TileSize;
				TileRect.h = TileSize;
				SDL_RenderDrawRect(Renderer, &TileRect);
#endif
			}
		}

		SDL_RenderPresent(Renderer);
		// }

		u32 WorkMSElapsedForFrame = SDL_GetTicks() - LastCounter;

		// NOTE(hugo) : Setting the window title
		char WindowTitle[128];
		sprintf(WindowTitle, "FastLife @ rivten - %ims", WorkMSElapsedForFrame);
		SDL_SetWindowTitle(Window, WindowTitle);

		if(WorkMSElapsedForFrame < TargetMSPerFrame)
		{
			u32 SleepMS = TargetMSPerFrame - WorkMSElapsedForFrame;
			if(SleepMS > 0)
			{
				SDL_Delay(SleepMS);
			}
		}
		else
		{
			// TODO(hugo) : Missed framerate
		}

		// NOTE(hugo) : This must be at the very end
		LastCounter = SDL_GetTicks();
	}

	Free(Universes);
	SDL_DestroyRenderer(Renderer);
	SDL_DestroyWindow(Window);
	return(0);
}
