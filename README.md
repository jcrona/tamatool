# TamaTool - A Cross-Platform Explorer for First-Gen Tamagotchi


## Synopsis

TamaTool is a cross-platform explorer for first-gen Tamagotchi (like P1 and P2) relying on the hardware agnostic Tamagotchi P1 emulation library [TamaLIB](https://github.com/jcrona/tamalib/), and mainly targeting Linux, Windows and MacOS.

More than a simple emulator, it is an exploration tool featuring a realtime RAM editor, an ASM debugger, an I/Os monitor and a sprite manipulator, allowing to play around with E0C6S46/8 based Tamagotchi ROMs. It also allows to save and restore its emulation state, thus allowing you to backup and share your best friend !

![TamaTool](misc/screenshot.png)![TamaTool-Shell](misc/screenshot2.png)

In order to create the background images used in TamaTool, I scanned the background of my own P1 and P2, that I filtered/enhanced using GIMP. The original scans can be found in the __misc__ folder. The shell is also a photo of my P1.

__Notes regarding the supported platforms__

The Windows build does not include the realtime memory editor. The ASM debugger and I/Os monitor do work, but the emulation is very slow when they are enabled because of the poor console performances.
Android support would be feasible, but a native implementation of TamaLIB without libSDL2 would probably be more efficient.

## Build instruction

First, you need to clone TamaTool and its submodule:
```
$ git clone --recursive https://github.com/jcrona/tamatool.git
```

Then the instructions depend on the targetted platform.

### Linux (or other Unix environments providing the dependencies as system libraries)

TamaTool depends on libSDL2, libSDL2-image and libpng.
On Ubuntu, you can install those libraries using the following command:
```
$ sudo apt-get install libsdl2-dev libsdl2-image-dev libpng-dev
```

Then you can build a distribution package (no library bundled) with:
```
$ make linux
```

The package will be available in the __linux__ folder.

### Windows

Only cross-compiling from linux using MinGW64 is supported. The required dependencies are provided as prebuilt binaries.
On Ubuntu you can install MinGW64 using the following command:
```
$ sudo apt install gcc-mingw-w64
```

Then you can build a distribution package (with the required libraries bundled) with:
```
$ make windows
```

The package will be available in the __windows__ folder.

### MacOS

TamaTool needs to be built on MacOS. The required dependencies are provided as prebuilt binaries.
You can build a distribution package (with the required libraries bundled) with:
```
$ make mac
```

The package will be available in the __mac__ folder.


## Usage

TamaTool being an emulator, it requires a compatible Tamagotchi ROM called __rom.bin__ in its folder. This ROM is not provided here.
For your information, the expected ROM format is 16 bits in big-endian per instruction (the actual E0C6S46 instructions are 12-bit long).

Hatching a new Tamagotchi:
```
$ ./tamatool
```

Using a specific ROM:
```
$ ./tamatool -r some_path/my_rom.bin
```

Starting the memory editor:
```
$ ./tamatool -e
```

Looking at the CPU instructions in realtime:
```
$ ./tamatool -c
```

Extracting the data from the ROM to a PNG file:
```
$ ./tamatool -E data.png
```

Importing back the data into the ROM:
```
$ ./tamatool -M data.png
```

When playing around with the extracted data, you can safely modify the sprites. However, modifying other data will likely result in a broken ROM.

Getting all the supported options:
```
$ ./tamatool -h
```

Once running, you can either click on the shell buttons to interact with the Tamagotchi, or you can press the __left__, __down__ and __right__ arrow keys of your keyboard.  
Pressing __s__ puts the emulation in step by step mode and executes the next instruction, while __r__ resumes it.  
Pressing __w__ fully executes the next instruction or call (no step by step inside a call).  
Pressing __x__ stops the execution right after the next call.  
Pressing __c__ stops the execution right after the next return.  
Pressing __f__ toggles between the original speed, x10 speed and unlimited speed.  
Pressing __t__ shows/hides the shell of the Tamagotchi.  
Pressing __i__ increases the size of the GUI, while __d__ decreases it.  
Pressing __b__ saves the emulation state to a __<rom_basename>_saveN.bin__ file, while __n__ loads the last saved state.


## License

TamaTool is distributed under the GPLv2 license. See the LICENSE file for more information.


## Hardware information

The Tamagotchi P1 and P2 are based on an E0C6S46 Epson MCU, and runs at 32,768 kHz, while the Tamagotchi Angel, Umino (Ocean), Morino (Forest), Mothra, Tamaotch and Genjintch are based on an E0C6S48 Epson MCU running at 32,768 kHz and 1 MHz. Their LCDs are all 32x16 B/W pixels, with 8 icons. 
To my knowledge, the P1 ROM available online has been extracted from a high-res picture of a die. The ROM mask was clear enough to be optically read. The pictures can be seen [there](https://siliconpr0n.org/map/bandai/tamagotchi-v1/) (thx asterick for the link!).  
~~I would love to see the same work done on a P2 and add support for it in TamaLIB/TamaTool!~~ Someone did the same job for the P2 and other first-gen devices!

__  
Copyright (C) 2025 Jean-Christophe Rona
