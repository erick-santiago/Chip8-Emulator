/**
 * @brief  CHIP8 EMULATOR PROJECT
 * @Author esantiago
 * @date   May, 2018
 *
 * @description: Main() source file for chip8 project
 */

#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include "Chip8.h"
#include "EmuGfx.h"

using namespace std;


int main( int argc, char *argv[] )
{

  Chip8 chip8_emu;
  EmuGfx chip8_Gfx;  


  // start up SDL and create window
  if( !chip8_Gfx.init() )
  {
	printf( "\nFailed to initialize render system!\n" );
  }

  else
  {
    // initialize Chip8 system 
    chip8_emu.initialize();
 
    // load game into memory
    if ( !chip8_emu.loadGame( argv[1] )  )
    {
      printf( "\nFailed to load media!\n" );
    }

    else
    {		
      // main loop flag
	  bool quit = false;

	  // event handler
	  SDL_Event e;

      // start background music
      Mix_PlayMusic( chip8_Gfx.bgMusic, -1);

	  //While application is running
	  while( !quit )
      {
          // emulate one cycle
          chip8_emu.emulateCycle();

          // if draw flag is set, update screen
          if (chip8_emu.draw_flag)
            chip8_Gfx.drawGfx( chip8_emu );

		  //printf( "\nBefore handle events in queue...\n" );
          // handle events in queue
		  while( SDL_PollEvent( &e ) != 0 )
		  {
			  //User requests quit
			  if ( e.type == SDL_QUIT )
			  {
				  quit = true;
			  }
 
              else if ( e.type == SDL_KEYDOWN )
              {
                for (size_t i = 0; i < 16; ++i) 
                {
                  if ( e.key.keysym.sym == chip8_Gfx.keymap[i] )
                  {
                    chip8_emu.key[i] = 1;
                    //printf( "\nAfter handle SDL_KEYDOWN...\n" );
                  }
                }
              }

              else if ( e.type == SDL_KEYUP )
              {
                for (size_t i = 0; i < 16; ++i) 
                {
                  if ( e.key.keysym.sym == chip8_Gfx.keymap[i] )
                    chip8_emu.key[i] = 0;
                  //printf( "\nAfter handle SDL_KEYUP...\n" );
                }
              }               
          //printf( "\nAfter handle events in queue...\n" );
		  }

   	  }

    }

  }

  // free resources and close SDL
  chip8_Gfx.close();

  return 0;

}

  


