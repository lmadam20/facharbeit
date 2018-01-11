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

#ifndef MACHINE_H
#define MACHINE_H

#include <config.standard.h>
#include <stdafx.h>
#include <SDL2/SDL.h>
#include "cpu.h"
#include "sgpu.h"

class Machine
{
public:
	/**
	 * Constructor of our machine.
     */
	Machine();

	/**
	 * Initializes the machine by initializing the CPU
     * and loading both bootrom/bios and rom into memory
     */
	int init();

	/**
	 * Starts the machine
     */
	int run();

	void setRomName(string rom_name);

	string getRomName();

	void WriteMem(dword address, byte value);

	byte ReadMem(dword address);

	void WriteIO(dword address, byte value);

	byte ReadIO(dword address);

	int GetClockFrequency();

	void Cycle();

	~Machine();

private:

	CPU* cpu;

	string rom_name;

	byte zeropage[0x0100]; // 256 bytes of memory mapped from 0x0 to 0x00ff
	byte ram[RAM_SIZE];
	byte bootrom_0[BOOTROM_0_SIZE]; // BootROM page 0
	byte rom_0[ROM_0_SIZE]; // ROM page 0

	byte* bootrom;
	int bootrom_size;
	byte bootrom_page;
	byte* rom;
	int rom_size;
	byte rom_page;

	/* Simple graphics processing unit (SGPU) */
	SGPU* sgpu;

	/* Keyboard */
	byte* kbd_state;
	int kbd_char;
	char kbd_last;

	/* Timer 0 */
	byte t0_ctrl;
	dword t0_kcycles;
	dword t0_kcycles_counted;

	int clock_frequency;
	timespec cycle_spec;
	int running;
};

static Machine* instance;

#endif // MACHINE_H
