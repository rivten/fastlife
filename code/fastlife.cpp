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
global_variable u32 GlobalWindowWidth = 1024 + 256 + 16;
global_variable u32 GlobalWindowHeight = 512 + 128 + 64 + 4;
global_variable u32 TileSize = 1;
#if 0
global_variable u32 UniverseWidth = 1 << 12;
global_variable u32 UniverseHeight = 1 << 12;
#else
global_variable u32 UniverseWidth = 4 * GlobalWindowWidth;
global_variable u32 UniverseHeight = 4 * GlobalWindowHeight;
#endif
global_variable u32 CameraPosDelta = 20;

inline u8
GetUniverseTile(u8* Universe, u32 Index)
{
	u8 Result = 0;

	u8 UniverseBatch = Universe[Index / 8];
	Result = (UniverseBatch >> (Index % 8)) & 1;

	return(Result);
}

inline u8
GetNeighborCount(u8* Universe, u32 IndexX, u32 IndexY)
{
	u8 Count = 0;

	{
		s32 X = IndexX - 1;
		s32 Y = IndexY - 1;
		u32 Index = (X % UniverseWidth) + (Y % UniverseHeight) * UniverseWidth;
		u8 UniverseBatch = Universe[Index / 8];
		u8 Result = (UniverseBatch >> (Index % 8)) & 1;
		Count += Result;
	}

	{
		s32 X = IndexX;
		s32 Y = IndexY - 1;
		u32 Index = (X % UniverseWidth) + (Y % UniverseHeight) * UniverseWidth;
		u8 UniverseBatch = Universe[Index / 8];
		u8 Result = (UniverseBatch >> (Index % 8)) & 1;
		Count += Result;
	}

	{
		s32 X = IndexX + 1;
		s32 Y = IndexY - 1;
		u32 Index = (X % UniverseWidth) + (Y % UniverseHeight) * UniverseWidth;
		u8 UniverseBatch = Universe[Index / 8];
		u8 Result = (UniverseBatch >> (Index % 8)) & 1;
		Count += Result;
	}

	{
		s32 X = IndexX - 1;
		s32 Y = IndexY;
		u32 Index = (X % UniverseWidth) + (Y % UniverseHeight) * UniverseWidth;
		u8 UniverseBatch = Universe[Index / 8];
		u8 Result = (UniverseBatch >> (Index % 8)) & 1;
		Count += Result;
	}

	{
		s32 X = IndexX + 1;
		s32 Y = IndexY;
		u32 Index = (X % UniverseWidth) + (Y % UniverseHeight) * UniverseWidth;
		u8 UniverseBatch = Universe[Index / 8];
		u8 Result = (UniverseBatch >> (Index % 8)) & 1;
		Count += Result;
	}

	{
		s32 X = IndexX - 1;
		s32 Y = IndexY + 1;
		u32 Index = (X % UniverseWidth) + (Y % UniverseHeight) * UniverseWidth;
		u8 UniverseBatch = Universe[Index / 8];
		u8 Result = (UniverseBatch >> (Index % 8)) & 1;
		Count += Result;
	}

	{
		s32 X = IndexX;
		s32 Y = IndexY + 1;
		u32 Index = (X % UniverseWidth) + (Y % UniverseHeight) * UniverseWidth;
		u8 UniverseBatch = Universe[Index / 8];
		u8 Result = (UniverseBatch >> (Index % 8)) & 1;
		Count += Result;
	}

	{
		s32 X = IndexX + 1;
		s32 Y = IndexY + 1;
		u32 Index = (X % UniverseWidth) + (Y % UniverseHeight) * UniverseWidth;
		u8 UniverseBatch = Universe[Index / 8];
		u8 Result = (UniverseBatch >> (Index % 8)) & 1;
		Count += Result;
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

	random_series Entropy = RandomSeed(1234);

	u32 SquareCount = UniverseWidth * UniverseHeight;
	Assert(SquareCount % 8 == 0);
	u32 BatchCount = SquareCount / 8;
	u8* Universes = AllocateArray(u8, 2 * BatchCount);
	u8* CurrentUniverse = Universes;
	u8* NextUniverse = Universes + BatchCount;

	// NOTE(hugo): Universe random init
	for(u32 Index = 0; Index < BatchCount; ++Index)
	{
		CurrentUniverse[Index] = u8(RandomChoice(&Entropy, 256));
	}

	u32 CameraPosX = 0;
	u32 CameraPosY = 0;

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

		s32 KeyCount = 0;
		const u8* KeyboardState = SDL_GetKeyboardState(&KeyCount);

		if(KeyboardState[SDL_SCANCODE_UP])
		{
			CameraPosY -= CameraPosDelta;
		}
		if(KeyboardState[SDL_SCANCODE_DOWN])
		{
			CameraPosY += CameraPosDelta;
		}
		if(KeyboardState[SDL_SCANCODE_LEFT])
		{
			CameraPosX -= CameraPosDelta;
		}
		if(KeyboardState[SDL_SCANCODE_RIGHT])
		{
			CameraPosX += CameraPosDelta;
		}

		// NOTE(hugo): Update
		// {
		u32 UniverseX = 0;
		u32 UniverseY = 0;
		for(u32 Index = 0; Index < BatchCount; ++Index)
		{
			// NOTE(hugo): Copy all eight values
			NextUniverse[Index] = CurrentUniverse[Index];

			for(u8 BitIndex = 0; BitIndex < 8; ++BitIndex)
			{
				++UniverseX;
				if(UniverseX == UniverseWidth)
				{
					UniverseX = 0;
					++UniverseY;
				}
				u8 NeighborCount = GetNeighborCount(CurrentUniverse, UniverseX, UniverseY);
				u8 UniverseBatch = CurrentUniverse[Index];
				u8 TileMask = (1 << BitIndex);
				if((UniverseBatch & TileMask) == 0)
				{
					if(NeighborCount == 3)
					{
						NextUniverse[Index] |= (1 << BitIndex);
					}
				}
				else if(((UniverseBatch & TileMask) != 0) &&
						(NeighborCount < 2 || NeighborCount > 3))
				{
					NextUniverse[Index] &= ~(1 << BitIndex);
				}
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
		for(u32 Y = 0; Y < GlobalWindowHeight; Y += TileSize)
		{
			for(u32 X = 0; X < GlobalWindowWidth; X += TileSize)
			{
				s32 UniverseX = CameraPosX + (X / TileSize);
				s32 UniverseY = CameraPosY + (Y / TileSize);
				if(UniverseX >= 0 && u32(UniverseX) < UniverseWidth &&
						UniverseY >= 0 && u32(UniverseY) < UniverseHeight)
				{
					u32 SquareIndex = UniverseX + UniverseY * UniverseWidth;
					if(GetUniverseTile(CurrentUniverse, SquareIndex) == 1)
					{
						SDL_Rect TileRect = {};
						TileRect.x = X;
						TileRect.y = Y;
						TileRect.w = TileSize;
						TileRect.h = TileSize;
						SDL_RenderFillRect(Renderer, &TileRect);
					}
				}
				else
				{
					// TODO(hugo): Another possibility: draw a grey
					// background and draw in white each time 
					// there is no cell
					SDL_SetRenderDrawColor(Renderer, 100, 100, 100, 255);
					SDL_Rect TileRect = {};
					TileRect.x = X;
					TileRect.y = Y;
					TileRect.w = TileSize;
					TileRect.h = TileSize;
					SDL_RenderFillRect(Renderer, &TileRect);
					SDL_SetRenderDrawColor(Renderer, 0, 255, 0, 255);
				}
			}
		}

		SDL_RenderPresent(Renderer);
		// }

		u32 WorkMSElapsedForFrame = SDL_GetTicks() - LastCounter;

		float GenerationPerSecond = 1000.0f / float(WorkMSElapsedForFrame);
		u32 CellComputationsPerSecond = u32(GenerationPerSecond * UniverseWidth * UniverseHeight);
		u32 MillionCellComputationsPerSecond = CellComputationsPerSecond / 1000000;

		// NOTE(hugo) : Setting the window title
		char WindowTitle[128];
		sprintf(WindowTitle, "FastLife @ rivten - %ims - %i (%iM) cell-computed per seconds", WorkMSElapsedForFrame,
				CellComputationsPerSecond, MillionCellComputationsPerSecond);
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
