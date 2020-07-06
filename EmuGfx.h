/**
 * @brief  CHIP8 EMULATOR PROJECT
 * @Author esantiago
 * @date   May, 2018
 *
 * @description: Header file for chip8 graphics utilizing SDL2 framework
 */
 
#ifndef EMUGFX_H_
#define EMUGFX_H_


class EmuGfx{

  public:

    EmuGfx();
    ~EmuGfx();

    // starts up SDL and creates window
    bool init();

    // update screen if draw flag is set
    void drawGfx(Chip8 &myChip8);

    // frees media and quits SDL
    void close();

    // keypad keymap
    unsigned char keymap[16] = 
    {
      SDLK_x, // 0
      SDLK_1, // 1
      SDLK_2, // 2
      SDLK_3, // 3
      SDLK_q, // 4
      SDLK_w, // 5
      SDLK_e, // 6
      SDLK_a, // 7
      SDLK_s, // 8
      SDLK_d, // 9
      SDLK_z, // A
      SDLK_c, // B
      SDLK_4, // C
      SDLK_r, // D
      SDLK_f, // E
      SDLK_v, // F
    };

    // create audio object
    Mix_Music *bgMusic;

  private:

    // the window we'll be rendering to
    SDL_Window *gfxWindow;

    // create renderer for window
    SDL_Renderer *gfxRenderer;

    // create a texture for a rendering context
    SDL_Texture *gfxTexture;

    // screen dimensions constants
    const int SCREEN_WIDTH;
    const int SCREEN_HEIGHT;

    // temporary pixel buffer
    uint32_t gfxPixels[2048];

    // timeout used to slowdown emulation speed
    uint32_t timeout;

};

#endif // EMUGFX_H_
    

    



