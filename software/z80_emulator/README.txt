z80emu - A virtual machine emulating a Z80-like processor
-----------------------------------------------------------

This directory contains the source code for the emulator.


Compiling
----------
Before you can build z80emu, you need the following prerequisites:
* SDL2
* SDL2_ttf

Before you are going to compile the software, choose a ttf font
and copy it into font/ and set the appropriate file name at
the define FONT_DEFAULT_NAME in your config (located at include/).

Compiling:
$ make


Before you can run the emulator you also need:
* A ROM
* The BootROM

The default BootROM sources are located at bootrom_src/.
To compile the BootROM sources using the provided shell scripts,
you also need z80asm as well as rompacker, which code can be found
in <repo root>/java-tools/rom-packer and must be built to the same
directory as rompacker.jar.

Next you need a ROM. An example assembler file can be found in examples/.


License
---------
Copyright (c) 2016-2017 Leon Maurice Adam.

Z80 Emulator is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Z80 Emulator is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Z80 Emulator.  If not, see <http://www.gnu.org/licenses/>.
