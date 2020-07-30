## Chip8-Emulator
A Chip8 Emulator written in C++

![SPACE INVADERS](Media/INVADERS.png "SPACE INVADERS")
*SPACE INVADERS*


![WIPEOFF](Media/WIPEOFF.png "WIPEOFF")
*WIPEOFF*

CHIP-8 is an interpreted programming language orginally developed by Joseph Weisbecker. It was initially used on the COSMAC VIP and Telmac 1800 8-bit microcomputers in the mid-1970s. The CHIP-8 programs are run on a CHIP-8 virtual machine, the purpose of which was to allow video games to be more easily programmed on these computers (source wiki). 

## Pre-reqs

Requires Make and SDL2:
```
$ sudo apt-get update
$ sudo apt-get install make
```

For those who have Advanced Packaging Tool available (i.e., Ubuntu, Debian, etc), you'll want to search the apt-get cache and find the current SDL2 version to install.  Use the command:
```
$ apt-cache search libsdl2
```
You'll want to download the development version of SDL2 and SDL2-mixer (audio library for sound). As of last update of this README, the development package(s) are libsdl2-dev and libsdl2-mixer-dev.  Use command:
```
$ sudo apt-get install libsdl2-dev libsdl2-mixer-dev
```

## Cloning, compiling and running

Clone and Compile:
```
$ git clone https://github.com/erick-santiago/Chip8-Emulator.git
$ cd Chip8-Emulator
$ make
```

Run:
```
$ ./testing_Chip8 ROMs/ROM-NAME-HERE
(example: ./testing_Chip8 ROMs/WIPEOFF)
```
The ROMs are included in the `ROMs` directory.

## Keyboard Controls

The computers which originally used the Chip-8 Language had a 16-key hexadecimal keypad. Below is the mapping from the original keypad to your current (standard) keyboard.

| Original | Keyboard |
|----------|----------|
| 1 2 3 C  | 1 2 3 4  |
| 4 5 6 D  | Q W E R  |
| 7 8 9 E  | A S D F  |
| A 0 B F  | Z X C V  |

## Tested With
Linux Mint 18.3 Sylvia

## References
Resources used to bring this project to life, and special thanks:

- http://www.multigesture.net/articles/how-to-write-an-emulator-chip-8-interpreter/
- http://devernay.free.fr/hacks/chip8/C8TECH10.HTM
- https://lazyfoo.net/tutorials/SDL/

## License
This project is licensed under the terms of the MIT license.
