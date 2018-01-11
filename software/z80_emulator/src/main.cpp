/*
	Copyright (c) 2016-2017 Leon Maurice Adam.
	
	This file is part of Z80 Emulator.

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
*/

#include "machine.h"

int main(int argc, char* argv[])
{
	cout << "Z80 Emulator starting" << endl << endl;

	int ret = SDL_Init(SDL_INIT_EVERYTHING); // initialize SDL2
	if (ret)
	{
		cerr << "Failed to initialize SDL2: " << SDL_GetError() << endl;
		return -1;
	}

	string rom = "rom.bin"; // standard ROM name is 'rom.bin'

	for (int i = 1; i < argc; i++) // i = 1, because i[0] is the application name
	{
		rom = argv[i];
	}

	Machine* machine = new Machine();
	machine->setRomName(rom);
	if (machine->init())
	{
		cerr << "Failed to initialize machine" << endl;
		SDL_Quit();
		delete machine;
		return -1;
	}

	ret = machine->run();
	cout << "Machine execution ended with " << ret << endl;

	delete machine;
	SDL_Quit();

	return 0;
}
