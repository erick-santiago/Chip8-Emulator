/**
 * @brief  CHIP8 PROJECT | Test source file
 * @Author Erick Santiago (esaniago.dev@gmail.com)
 * @date   May, 2018
 *
 * @description: This file is the Test source file for the Chip8 emu project 
 */


#include <cstddef>
#include "Chip8.h"

using namespace std;


int main( int argc, char *argv[] )
{  

  Chip8 chip8_emu;

  chip8_emu.disassembler( argv[1] );

  return 0;  
  
}
