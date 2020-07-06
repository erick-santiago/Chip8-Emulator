/**
 * @brief  CHIP8 EMULATOR PROJECT 
 * @Author esantiago
 * @date   May, 2018
 *
 * @description: Header file for Chip8 specs
 *
 * The systems memory map:
 * 0x000 - 0x1FF - Chip-8 Interpreter (contains font set in emu)
 * 0x050 - 0x0A0 - Used for the built-in 4x5 pixel font set (0-F)
 * 0x200 - 0xFFF - Program ROM and work RAM
 */
 
#ifndef CHIP8_H_
#define CHIP8_H_


class Chip8{

  friend class EmuGfx;

  private:

    // CHIP-8 CPU Specs
    unsigned short opcode;
    unsigned char memory[4096];
    unsigned char V[16]; // 16 general purpose 8-bit registers. VF is used as a flag
    unsigned short I; // 16-bit register, generally used to store memory addresses
    unsigned short pc; // program counter
    unsigned char gfx[64 * 32]; // graphics buffer
    unsigned char delay_timer;
    unsigned char sound_timer;
    unsigned short stack[16];
    unsigned short sp; // stack pointer

    // helper members
    unsigned char *m_buffer;
    size_t m_size;
    size_t pos;

    // helper methods
    bool readIntoBuffer( const char *strFileName);
    void decoder( size_t pc);
    void buffer_deallocate();


    // Chip-8 Fontset
    // Should properly be stored in interpreter area
    // of Chip-8 memory (0x000 - 0x1FF)
    unsigned char Chip8_fontset[80];

  public:

    Chip8();
    ~Chip8();

    bool draw_flag;
    unsigned char key[16]; // simple HEX keypad

    void initialize();
    bool loadGame( const char *hexFile);
    void emulateCycle();
    void disassembler( const char *hexFile);

};

#endif // CHIP8_H_





