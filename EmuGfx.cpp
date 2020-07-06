#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include "stdint.h"
#include "Chip8.h"
#include "EmuGfx.h"

using namespace std;


EmuGfx::EmuGfx()
  : gfxWindow(NULL), gfxRenderer(NULL), gfxTexture(NULL), bgMusic(NULL), SCREEN_WIDTH(1024), SCREEN_HEIGHT(512) //640 x 480
{
}


EmuGfx::~EmuGfx() 
{
}


bool EmuGfx::init()
{
    // initialization flag
	bool success = true;

	// initialize SDL
	if( SDL_Init( SDL_INIT_EVERYTHING ) < 0 )
	{
		printf( "SDL could not initialize! SDL Error: %s\n", SDL_GetError() );
		success = false;
	}
	else
	{
		//set texture filtering to linear
		//if( !SDL_SetHint( SDL_HINT_RENDER_SCALE_QUALITY, "0" ) )
		//{
		//    printf( "Warning: Linear texture filtering not enabled!" );
		//}

		// create window
		gfxWindow = SDL_CreateWindow( "Quantum's Chip8 Emulator!", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN );
		if( gfxWindow == NULL )
		{
			printf( "Window could not be created! SDL Error: %s\n", SDL_GetError() );
			success = false;
		}
		else
		{
			// create renderer for window
			gfxRenderer = SDL_CreateRenderer( gfxWindow, -1, 0 );  // if want to Create vsynced renderer for window (see SDL_RendererFlags)
			if( gfxRenderer == NULL )
			{
				printf( "Renderer could not be created! SDL Error: %s\n", SDL_GetError() );
				success = false;
			}
			else
			{
				// initialize renderer color
				//SDL_SetRenderDrawColor( gfxRenderer, 0xFF, 0xFF, 0xFF, 0xFF );

                // Use this function to set a device independent resolution for rendering
                SDL_RenderSetLogicalSize( gfxRenderer, SCREEN_WIDTH, SCREEN_HEIGHT);

                // Use this function to create a texture for a rendering context
                gfxTexture = SDL_CreateTexture( gfxRenderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, 64, 32 );

                // initialize SDL Mixer and load audio file
                Mix_OpenAudio( 44100, MIX_DEFAULT_FORMAT, 2, 8192 );
                bgMusic = Mix_LoadMUS( "thruSpace.ogg" );
                
			}
		}
	}

	return success;
}


void EmuGfx::close()
{
    //Free audio
    Mix_FreeMusic( bgMusic );
    Mix_CloseAudio();

	//Free loaded texture
	SDL_DestroyTexture( gfxTexture );
	gfxTexture = NULL;

	//Destroy window	
	SDL_DestroyRenderer( gfxRenderer );
	SDL_DestroyWindow( gfxWindow );
	gfxWindow = NULL;
	gfxRenderer = NULL;

	//Quit SDL 
	SDL_Quit();
}


void EmuGfx::drawGfx(Chip8 &myChip8)
{
    myChip8.draw_flag = false;

    // store raw pixel data in format of texture in rendering buffer: gfxPixels[]
    for (size_t i = 0; i < 2048; ++i)
    {
      uint8_t pixel = myChip8.gfx[i];
      gfxPixels[i] = (0xFF0000FF * pixel);
    }

    // update texture
    SDL_UpdateTexture(gfxTexture, NULL, gfxPixels, 64 * sizeof(Uint32) );

    // clear screen
    SDL_RenderClear(gfxRenderer);

    // render texture to screen
    SDL_RenderCopy(gfxRenderer, gfxTexture, NULL, NULL);

    // update screen
    SDL_RenderPresent(gfxRenderer);

    // timeout used to slow down emulation speed
    // essentially rendering at ~130Hz
    timeout = SDL_GetTicks() + 8;
    while (!SDL_TICKS_PASSED( SDL_GetTicks(), timeout ) ) {};

}






