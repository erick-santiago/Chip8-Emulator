#include <iostream>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "Chip8.h"

using namespace std;


Chip8::Chip8() : m_buffer(NULL), draw_flag(false)
{
  // Chip-8 Fontset:
  // Programs may refer to group of sprites representing 
  // the hexadecimal digits 0 through F
  unsigned char Chip8_fontset[80] =
    { 
      0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
      0x20, 0x60, 0x20, 0x20, 0x70, // 1
      0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
      0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
      0x90, 0x90, 0xF0, 0x10, 0x10, // 4
      0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
      0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
      0xF0, 0x10, 0x20, 0x40, 0x40, // 7
      0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
      0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
      0xF0, 0x90, 0xF0, 0x90, 0x90, // A
      0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
      0xF0, 0x80, 0x80, 0x80, 0xF0, // C
      0xE0, 0x90, 0x90, 0x90, 0xE0, // D
      0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
      0xF0, 0x80, 0xF0, 0x80, 0x80  // F
    };

}


Chip8::~Chip8()
{
  delete[] m_buffer;
  m_buffer = NULL;
}


// Start clearing the memory and resetting the registers to zero
void Chip8::initialize()
{
  pc     = 0x200; // Program counter starts at 0x200
  opcode = 0;     // Reset current opcode
  I      = 0;     // Reset index register
  sp     = 0;     // Reset stack pointer

  // Clear display
  for (size_t i = 0; i < 2048; ++i)
    gfx[i] = 0;

  // Clear stack, registers V0 - VF and keypad  
  for (size_t i = 0; i < 16; ++i){
    stack[i] = 0;
    V[i]     = 0;
    key[i]   = 0;
  }
 
  // Clear memory
  for (size_t i = 0; i < 4096; ++i) 
    memory[i] = 0;

  // Load fontset
  for (size_t i = 0; i < 80; ++i)
    memory[i + 80] = Chip8_fontset[i];

  // Reset timers
  delay_timer = 0;
  sound_timer = 0;

  // Seeding rng
  srand(time(NULL));

}


bool Chip8::loadGame(const char *hexFile)
{
  // initialization flag
  bool success = true;

  if ( readIntoBuffer( hexFile) )
  {
    for (size_t i = 0; i < m_size; ++i)
      memory[i + 512] = m_buffer[i];
  }

  else
  { 
    printf( "\nFailed to read into buffer!\n" );
    success = false;
  }

  return success;

}


void Chip8::emulateCycle()
{
  // fetch opcode
  opcode = (memory[pc] << 8 | memory[pc+1]);

  // decode and exucute opcode
  switch(opcode & 0xF000)
  {
    case 0x0000:
      switch(opcode & 0x000F)
      {
        case 0x0000:  // 00E0 - CLS: Clear the display
          for (size_t i = 0; i < 2048; ++i)
            gfx[i] = 0;
          draw_flag = true; 
          pc += 2;
        break;

        case 0x000E:  // 00EE - RET: Return from a subroutine; Interpreter sets pc to the address at the top of the stack, then subtracks 1 from sp
          --sp;
          pc = stack[sp];
          pc += 2;
        break;

        default: 
          printf("Unknown opcode..."); 
        break;
      }
    break;

    case 0x1000:  // 1nnn - JP addr: Jump to location nnn; The interpreter sets the pc to nnn
      pc = (opcode & 0x0FFF);
    break;

    case 0x2000:  // 2nnn - CALL addr: call subroutine at nnn; The interpreter increments the sp, then puts the current pc on the top of the stack. The pc is then set to nnn
      stack[sp] = pc;
      ++sp;
      pc = (opcode & 0x0FFF);
    break;

    case 0x3000:  // 3xkk - SE Vx, byte: Skip next instruction if Vx = kk (increments pc by 2)
      if ( V[(opcode & 0x0F00) >> 8] == (opcode & 0x00FF) )
        pc += 4;
      else
        pc += 2;
    break;

    case 0x4000:  // 4xkk - SNE Vx, byte: Skip next instruction if Vx != kk (increments pc by 2)
      if ( V[(opcode & 0x0F00) >> 8] != (opcode & 0x00FF) )
        pc += 4;
      else
        pc += 2;
    break;

    case 0x5000:  // 5xy0 - SE Vx, Vy: Skip next instruction if Vx = Vy (increments pc by 2)
      if ( V[(opcode & 0x0F00) >> 8] == V[(opcode & 0x00F0) >> 4] )
        pc += 4;
      else
        pc += 2;
    break;

    case 0x6000:  // 6xkk - LD Vx, byte: Set Vx = kk; The interprester puts the value kk into register Vx
      V[(opcode & 0x0F00) >> 8] = (opcode & 0x00FF);
      pc += 2;
    break;

    case 0x7000:  // 7xkk - ADD Vx, byte: Set Vx = Vx + kk; Adds the value kk to the value of register Vx, then stores the result in Vx
      V[(opcode & 0x0F00) >> 8] += (opcode &0x00FF);
      pc += 2;
    break;

    case 0x8000:
      switch(opcode & 0x000F)
      {
        case 0x0000:  // 8xy0 - LD Vx, Vy: Set Vx = Vy; Stores the value of register Vy in register Vx
          V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4];
          pc += 2;
        break;

        case 0x0001:  // 8xy1 - OR Vx, Vy: Set Vx = Vx OR Vy; Performs a bitwise OR on the values of Vx and Vy, then stores the result in Vx 
          V[(opcode & 0x0F00) >> 8] |= V[(opcode & 0x00F0) >> 4];
          pc += 2;
        break;

        case 0x0002:  // 8xy2 - AND Vx, Vy: Set Vx = Vx AND Vy; Performs a bitwise AND on the values of Vx and Vy, then stores the result in Vx 
          V[(opcode & 0x0F00) >> 8] &= V[(opcode & 0x00F0) >> 4];
          pc += 2;
        break;

        case 0x0003:  // 8xy3 - XOR Vx, Vy: Set Vx = Vx XOR Vy; Performs a bitwise exclusive OR on the values of Vx and Vy, then stores the result in Vx
          V[(opcode & 0x0F00) >> 8] ^= V[(opcode & 0x00F0) >> 4];
          pc += 2;
        break;

        case 0x0004:  // 8xy4 - Add Vx, Vy: Set Vx = Vx + Vy, set VF = carry; The values of Vx and Vy are added together
                      // If the result is greater than 8 bits (i.e., > 255) VF is set to 1, otherwise 0. Only lowest 8 bits of result are kept, and stored in Vx 
          if ( V[(opcode & 0x00F0) >> 4] > (0xFF - V[(opcode & 0x0F00) >> 8]) )
            V[0xF] = 1; // carry
          else
            V[0xF] = 0;
          V[(opcode & 0x0F00) >> 8] += V[(opcode & 0x00F0) >> 4];
          pc += 2;
        break;

        case 0x0005:  // 8xy5 - SUB Vx, Vy: Set Vx = Vx - Vy, set VF = NOT borrow; If Vx > Vy, then VF is set to 1, otherwise 0. Then Vy is subtracted from Vx, and result stored in Vx
          if ( V[(opcode & 0x00F0) >> 4] > V[(opcode & 0x0F00) >> 8] )
            V[0xF] = 0; // borrow
          else
            V[0xF] = 1; // NOT borrw
          V[(opcode & 0x0F00) >> 8] -= V[(opcode & 0x00F0) >> 4];
          pc += 2;
        break;

        case 0x0006:  // 8xy6 - Vx = Vx >> 1: Shifts Vx right by one and stores result in Vx. VF is set to value of least significant bit of Vx before the shift
          V[0xF] = ( V[(opcode & 0x0F00) >> 8] & 0x1);
          V[(opcode & 0x0F00) >> 8] >>= 1;
          pc += 2;
        break;

        case 0x0007:  // 8xy7 - SUBN Vx, Vy: Set Vx = Vy - Vx, set VF = NOT borrow; If Vy > Vx, then VF is set to 1, otherwise 0. Then Vx is subtracted from Vy, and result stored in Vx
          if ( V[(opcode & 0x0F00) >> 8] > V[(opcode & 0x00F0) >> 4] )
            V[0xF] = 0; // borrow
          else
            V[0xF] = 1; // NOT borrow
          V[(opcode & 0x0F00) >> 8] = ( V[(opcode & 0x00F0) >> 4] - V[(opcode & 0x0F00) >> 8] );
          pc += 2;
        break;

        case 0x000E:  // 8xyE - Vx = Vy << 1: Shifts Vx left by one and stores result in Vx. VF is set to the value of most significant bit of Vx before the shift
          V[0xF] = V[(opcode & 0x0F00) >> 8] >> 7;
          V[(opcode & 0x0F00) >> 8] <<= 1;
          pc += 2;
        break;

        default:  
          printf("Unknown opcode..."); 
        break;
      }
    break;

    case 0x9000:  // 9xy0 - SNE Vx, Vy: Skip next instruction if Vx !=  Vy (increments pc by 2)
      if ( V[(opcode & 0x0F00) >> 8] != V[(opcode & 0x00F0) >> 4] )
        pc += 4;
      else
        pc += 2;
    break;

    case 0xA000:  // Annn - LD I, addr: Set I = nnn; The value of register I is set to nnn
      I = (opcode & 0x0FFF);
      pc += 2;
    break;

    case 0xB000:  // Bnnn - JP V0, addr: Jump to location nnn + V0; The pc is set to nnn plus the value of V0
      pc = (opcode & 0x0FFF) + V[0];
    break;

    case 0xC000:  // Cxkk - RND Vx, byte: Set Vx = random byte AND kk; The interpreter generates a random number from 0 - 255, which is then ANDed with the value kk. The result is stored in Vx
      V[(opcode & 0x0F00) >> 8] = (rand() % (0xFF + 1)) & (opcode & 0x00FF);
      pc += 2;
    break;

    case 0xD000:  // Dxyn - DRW Vx, Vy, nibble: The interpreter reads and displays n-byte sprite starting at memory location I at (Vx,Vy), set VF = collision
    {             // Sprites are XORed onto the existing screen. If this causes any pixels to be erased, VF is set to 1, otherwise it is set to 0 
      unsigned char x = V[(opcode & 0x0F00) >> 8];
      unsigned char y = V[(opcode & 0x00F0) >> 4];
      unsigned char height  = (opcode & 0x000F);
      unsigned char pixel;

      V[0xF] = 0;
      for (size_t yline = 0; yline < height; yline++)
      {
        pixel = memory[I + yline];
        for(size_t xline = 0; xline < 8; xline++)
        {
          if((pixel & (0x80 >> xline)) != 0)
          {
            if(gfx[(x + xline + ((y + yline) * 64))] == 1) // The '64' is to be able to land on the correct row on the 64x32 original display
              V[0xF] = 1;                                 
            gfx[x + xline + ((y + yline) * 64)] ^= 1;
          }
        }
      }
 
      draw_flag = true;
      pc += 2;

    }  
    break;

    case 0xE000:
      switch(opcode & 0x000F)
      {
        case 0x000E:  // Ex9E - SKP Vx: Skips the next instruction if the key stored in Vx is pressed (checks keyboard), pc is increased by 2
          if (key[V[(opcode & 0x0F00) >> 8]] != 0)
            pc += 4;
          else
            pc += 2; 
        break;

        case 0x0001:  // ExA1 - SKNP Vx: Skips the next instruction if the key stored in Vx isn't pressed (checks keyboard), pc is increased by 2
          if (key[V[(opcode & 0x0F00) >> 8]] == 0)
            pc += 4;
          else
            pc += 2;
        break;
      }
    break;

    case 0xF000:
      switch(opcode & 0x00FF)
      {
        case 0x0007:  // Fx07 - LD Vx, DT: Set Vx = delay timer value. The value of DT is placed into Vx
          V[(opcode & 0x0F00) >> 8] = delay_timer;
          pc += 2;
        break;

        case 0x000A:  // Fx0A - LD Vx, K: Wait for a key press, store value of the key in Vx; All execution stops until a key is pressed, then value of that key is stored in Vx
        {
          bool key_press = false;
          for(size_t i = 0; i < 16; ++i)
          {
            if(key[i] != 0)
            {
              V[(opcode & 0x0F00) >> 8] = i;
              key_press = true;
            }
          }
          if(!key_press)
            return;
          pc += 2; 
        }
        break;

        case 0x0015:  // Fx15 - LD DT, Vx: Set delay timer = Vx; DT is set equal to value of Vx
          delay_timer = V[(opcode & 0x0F00) >> 8];
          pc += 2;
        break;

        case 0x0018:  // Fx18 - LD ST, Vx: Set sound timer = Vx; ST is set equal to value of Vx
          sound_timer = V[(opcode & 0x0F00) >> 8];
          pc += 2;
        break;

        case 0x001E:  // Fx1E - ADD I, Vx: Set I = I + Vx; The value of I and Vx are added, and result is stored in I 
          if(I + V[(opcode & 0x0F00) >> 8] > 0xFFF)
            V[0xF] = 1;
          else
            V[0xF] = 0;
          I += V[(opcode & 0x0F00) >> 8];
          pc += 2;
        break;

        case 0x0029:  // Fx29 - LD I, Vx: Sets I to the location of the sprite for the character in Vx; Characters 0-F (in hex) are respresented by a 4x5 font.
          I = V[(opcode & 0x0F00) >> 8] * 0x5 + 0x50; // System memory map: 0x050 - 0x0A0 - Used for the built-in 4x5 pixel font set (0-F)
          pc += 2;
        break;

        case 0x0033:  // Fx33 - LD [I], Vx: Interpreter takes the decimal value of Vx, and places the hundreds digit in memory at location in I, tens digit at location I+1, ones digit I+2 
          memory[I] = V[(opcode & 0x0F00) >> 8] / 100;
          memory[I+1] = (V[(opcode & 0x0F00) >> 8] %100) / 10;
          memory[I+2] =  V[(opcode & 0x0F00) >> 8] %10;
          pc += 2;
        break;

        case 0x0055:  // Fx55 - LD [I], Vx: The interpreter copies the values of registers V0 through Vx into memory, starting at address in I. I is set to I + X + 1 afer operation.
          for (size_t i = 0; i <= ((opcode & 0x0F00) >> 8); ++i)
            memory[I+i] = V[i];
          I += ( (opcode & 0x0F00) >> 8 ) + 1;
          pc += 2;
        break;

        case 0x0065:  // Fx65 - LD Vx, [I]: The interpreter fills V0 to Vx with values from memory starting at address I. I is set to I + X + 1 afer operation.
          for (size_t i = 0; i <= ((opcode & 0x0F00) >> 8); ++i)
            V[i] = memory[I+i];
          I += ( (opcode & 0x0F00) >> 8 ) + 1;
          pc += 2;
        break;
      }
    break;

    default: printf("Unknown opcode..."); break;

  }

  // update timers
  if(delay_timer > 0)
    --delay_timer;
  if(sound_timer > 0)
  {
    if(sound_timer == 1)
    {
      //printf("Beep!\n");
    }
    --sound_timer;    

  }

}


void Chip8::disassembler( const char *hexFile)
{

  if ( readIntoBuffer( hexFile) )
  {
    pos = 0;

    while (pos < m_size)
    {
      decoder( pos);
      pos += 2;
      printf ("\n");
    }

    buffer_deallocate();
  }

  else 
    printf( "\nFailed to read into buffer!\n" );

}


bool Chip8::readIntoBuffer( const char *strFileName) 
{
  // initialization flag
  bool success = true;

  streampos size;

  ifstream file(strFileName, ios::in|ios::binary|ios::ate);
  if (file.is_open())
  {
    if (m_buffer)
      buffer_deallocate();
    size = file.tellg();
    m_size = size;
    m_buffer = new unsigned char[size];
    if (m_buffer)
    {
      cout << "Allocation successful!" << endl;
      file.seekg(0,ios::beg);
      file.read((char *)m_buffer, size);
      file.close();
      cout << "\nThe file content is now in memory..." << endl;
    }

    else
    {
      cout << "\nAllocation failed!" << endl;
      success = false;
    }
  }

  else
  {
    cout << "\nUnable to open file..." << endl;
    success = false;
  }

  return success;

}


// Used as debugger and to study any ROM that's written in the Chip-8 language
void Chip8::decoder( size_t pc) 
{
  opcode = (m_buffer[pc] << 8 | m_buffer[pc+1]);

  printf("%03lu %X ", pc, opcode);
  switch(opcode & 0xF000)
  {
    case 0x0000:
      switch(opcode & 0x000F)
      {
        case 0x0000: printf("00E0 - CLS: Clear the display"); break;

        case 0x000E: printf("00EE - RET: Return from a subroutine; Interpreter sets pc to the address at the top of the stack, then subtracks 1 from sp."); break;

        default: printf("Unknown opcode..."); break;
      }
    break;

    case 0x1000: printf("1nnn - JP addr: Jump to location nnn; The interpreter sets the pc to nnn."); break;

    case 0x2000: printf("2nnn - CALL addr: call subroutine at nnn; The interpreter increments the sp, then puts the current pc on the top of the stack. The pc is then set to nnn."); break;

    case 0x3000: printf("3xkk - SE Vx, byte: Skip next instruction if Vx = kk (increments pc by 2)"); break;

    case 0x4000: printf("4xkk - SNE Vx, byte: Skip next instruction if Vx != kk (increments pc by 2)"); break;

    case 0x5000: printf("5xy0 - SE Vx, Vy: Skip next instruction if Vx = Vy (increments pc by 2)"); break;

    case 0x6000: printf("6xkk - LD Vx, byte: Set Vx = kk; The interprester puts the value kk into register Vx."); break;

    case 0x7000: printf("7xkk - ADD Vx, byte: Set Vx = Vx + kk; Adds the value kk to the value of register Vx, then stores the result in Vx."); break;

    case 0x8000:
      switch(opcode & 0x000F)
      {
        case 0x0000: printf("8xy0 - LD Vx, Vy: Set Vx = Vy; Stores the value of register Vy in register Vx"); break;

        case 0x0001: printf("8xy1 - OR Vx, Vy: Set Vx = Vx OR Vy; Performs a bitwise OR on the values of Vx and Vy, then stores the result in Vx."); break;

        case 0x0002: printf("8xy2 - AND Vx, Vy: Set Vx = Vx AND Vy; Performs a bitwise AND on the values of Vx and Vy, then stores the result in Vx."); break;

        case 0x0003: printf("8xy3 - XOR Vx, Vy: Set Vx = Vx XOR Vy; Performs a bitwise exclusive OR on the values of Vx and Vy, then stores the result in Vx."); break;

        case 0x0004: printf("8xy4 - Add Vx, Vy: Set Vx = Vx + Vy, set VF = carry; The values of Vx and Vy are added together. ");
                     printf("\nIf the result is greater than 8 bits (i.e., > 255) VF is set to 1, otherwise 0. Only lowest 8 bits of result are kept, and stored in Vx."); break;

        case 0x0005: printf("8xy5 - SUB Vx, Vy: Set Vx = Vx - Vy, set VF = NOT borrow; If Vx > Vy, then VF is set to 1, otherwise 0. Then Vy is subtracted from Vx, and result stored in Vx."); break;

        case 0x0006: printf("8xy6 - Vx = Vy >> 1: Shifts Vy right by one and stores result in Vx (Vy remains unchanged). VF is set to value of least significant bit of Vy before the shift."); break;

        case 0x0007: printf("8xy7 - SUBN Vx, Vy: Set Vx = Vy - Vx, set VF = NOT borrow; If Vy > Vx, then VF is set 1, otherwise 0. Then Vx is subtracted from Vy, and result stored in Vx."); break;

        case 0x000E: printf("8xyE - SHL Vx {, Vy}: Set Vx = Vx SHL 1; If the most-signigicant bit of Vx is 1, then VF is set to 1, otherwise 0. Then Vx is multiplied by 2."); break;
      }
    break;

    case 0x9000: printf("9xy0 - SNE Vx, Vy: Skip next instruction if Vx !=  Vy (increments pc by 2)"); break;

    case 0xA000: printf("Annn - LD I, addr: Set I = nnn; The value of register I is set to nnn."); break;

    case 0xB000: printf("Bnnn - JP V0, addr: Jump to location nnn + V0; The pc is set to nnn plus the value of V0."); break;

    case 0xC000: printf("Cxkk - RND Vx, byte: Set Vx = random byte AND kk; The interpreter generates a random number from 0 - 255, which is then ANDed with the value kk. The result is stored in Vx."); break;

    case 0xD000: printf("Dxyn - DRW Vx, Vy, nibble: The interpreter reads and displays n-byte sprite starting at memory location I at (Vx,Vy), set VF = collision.");
                 printf("\nSprites are XORed onto the existing screen. If this causes any pixels to be erased, VF is set to 1, otherwise it is set to 0."); break;

    case 0xE000:
      switch(opcode & 0x000F)
      {
        case 0x000E: printf("Ex9E - SKP Vx: Skip next instruction if key with the value of Vx is pressed (checks keyboard), pc is increased by 2."); break;

        case 0x0001: printf("ExA1 - SKNP Vx: Skip next instruction if key with the value of Vx is not pressed (checks keyboard), pc is increased by 2."); break;
      }
    break;

    case 0xF000:
      switch(opcode & 0x00FF)
      {
        case 0x0007: printf("Fx07 - LD Vx, DT: Set Vx = delay timer value. The value of DT is placed into Vx."); break;

        case 0x000A: printf("Fx0A - LD Vx, K: Wait for a key press, store value of the key in Vx; All execution stops until a key is pressed, then value of that key is stored in Vx."); break;

        case 0x0015: printf("Fx15 - LD DT, Vx: Set delay timer = Vx; DT is set equal to value of Vx."); break;

        case 0x0018: printf("Fx18 - LD ST, Vx: Set sound timer = Vx; ST is set equal to value of Vx."); break;

        case 0x001E: printf("Fx1E - ADD I, Vx: Set I = I + Vx; The value of I and Vx are added, and result is stored in I."); break;

        case 0x0029: printf("Fx29 - LD I, Vx: Set I = location of sprite for digit Vx; The value of I is set to location for hexadecimal sprite corresponding to the value of Vx."); break;

        case 0x0033: printf("Fx33 - LD B, Vx: Interpreter takes the decimal value of Vx, and places the hundreds digit in memory at location in I, tens digit at location I+1, ones digit I+2."); break;

        case 0x0055: printf("Fx55 - LD [I], Vx: The interpreter copies the values of registers V0 through Vx into memory, starting at address in I."); break;

        case 0x0065: printf("Fx65 - LD Vx, [I]: The interpreter fills V0 to Vx with values from memory starting at address I."); break;
      }
    break;

    default: printf("Unknown opcode..."); break;

  }

}


void Chip8::buffer_deallocate()
{
  delete [] m_buffer;
  m_buffer = NULL;

}



