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

Machine::Machine()
{
	this->rom_name = "rom.bin";
	instance = this;
	clock_frequency = CLOCK_FREQUENCY;
	running = 1;
	cycle_spec.tv_sec = 0;
	cycle_spec.tv_nsec = 1 / (GetClockFrequency() / 1000000) * 1000;
	t0_kcycles_counted = 0;
	t0_ctrl = 0;
	cpu = NULL; // avoid segmentation fault when trying to delete CPU
}

void Machine::setRomName(string rom_name)
{
	this->rom_name = rom_name;
}

string Machine::getRomName()
{
	return rom_name;
}

int Machine::init()
{
	/* Marking zeropage (for debugging purposes) */
	memset(zeropage, 0xAA, sizeof(zeropage));

	/* BootROM / BIOS loading */
	cout << "Loading BootROM '" << BOOTROM_FILENAME << "'..." << endl;

	ifstream bootrom_file(BOOTROM_FILENAME, ifstream::in | ifstream::binary | ifstream::ate);
	if (!bootrom_file.is_open())
	{
		cerr << "Unable to load BootROM!" << endl;
		return -1;
	}

	this->bootrom_size = bootrom_file.tellg();
	if (this->bootrom_size == 0)
	{
		cout << "note: BootROM is empty" << endl;
	}
	else
	{
		this->bootrom = new byte[this->bootrom_size];
		bootrom_file.seekg(0, ifstream::beg);
		bootrom_file.read((char*)this->bootrom, this->bootrom_size);
	}
	bootrom_file.close();
	cout << endl;


	/* ROM loading */
	cout << "Loading ROM '" << rom_name << "'..." << endl;

	ifstream rom_file(rom_name.c_str(), ifstream::in | ifstream::binary | ifstream::ate);
	if (!rom_file.is_open())
	{
		cerr << "Unable to load ROM!" << endl;
		return -1;
	}

	this->rom_size = rom_file.tellg();
	if (this->rom_size == 0)
	{
		cout << "note: ROM is empty" << endl;
	}
	else
	{
		this->rom = new byte[this->rom_size];
		rom_file.seekg(0, ifstream::beg);
		rom_file.read((char*)this->rom, this->rom_size);
	}
	rom_file.close();
	cout << endl;

	/* CPU initialization */
	cpu = new CPU();
	cpu->printState();

	/* SGPU init */
	sgpu = new SGPU(FB_N_OFFSET);
	sgpu->init(640, 480);
	sgpu->initFB0(FB_WIDTH, FB_HEIGHT);

	/* Keyboard */
	kbd_state = new byte[255];

	return 0;
}

int Machine::run()
{	
	while (running)
	{
		/* Keyboard/event handling */
		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
			if (event.type == SDL_QUIT)
			{
				running = 0;
				break;
			}
			else if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP)
			{
				int val = event.key.state == SDL_PRESSED ? 1 : 0;
				int key = event.key.keysym.sym;

				if (key <= 255 && key != 0) kbd_state[key] = val;
				else if (key == SDLK_LSHIFT || key == SDLK_RSHIFT) kbd_state[0] = val; // map shift key to keycode 0

				if (key == kbd_last && !val) kbd_last = 0; // key in kbd_last has been released => 0
				else if (val) kbd_last = key; // set kbd_last to new key
			}
		}

		cpu->next();

		/* Timer handling */
		if (t0_kcycles_counted >= t0_kcycles && (t0_ctrl & T0_CTRL_ENABLE))
		{
			t0_ctrl |= T0_CTRL_TRIGGER;
			t0_kcycles_counted = 0;
			if ((t0_ctrl & T0_CTRL_ENABLE_IRQ))
				cpu->triggerIRQ();
		}
		else if (!(t0_ctrl & T0_CTRL_ENABLE))
		{
			t0_kcycles_counted = 0;
		}

		/* Rendering */
		sgpu->render();
	}
	delete sgpu;
	return 0;
}

Machine::~Machine()
{
	if (cpu != NULL) delete cpu; // free only when CPU has been created with new
}

void Machine::WriteMem(dword address, byte value)
{
	if (address < 0x0100)
	{
		zeropage[address] = value;
	}
	else if (address >= RAM_OFFSET && address < (RAM_OFFSET + RAM_SIZE))
	{
		ram[address - RAM_OFFSET] = value;
	}
	else if (address >= FB_N_OFFSET && address < (FB_N_OFFSET + FB_N_SIZE))
	{
		sgpu->writeFB(address, value); // pass over to SGPU
	}
}

byte Machine::ReadMem(dword address)
{
	if (address < 0x0100)
	{
		return zeropage[address];
	}
	else if (address >= RAM_OFFSET && address < (RAM_OFFSET + RAM_SIZE))
	{
		return ram[address - RAM_OFFSET];
	}
	else if (address >= FB_N_OFFSET && address < (FB_N_OFFSET + FB_N_SIZE))
	{
		return sgpu->readFB(address); // pass over to SGPU
	}
	else if (address >= BOOTROM_0_OFFSET && address < (BOOTROM_0_OFFSET + BOOTROM_0_SIZE))
	{
		if ((address - BOOTROM_0_OFFSET) >= bootrom_size)
			return 0;
		return bootrom[address - BOOTROM_0_OFFSET];
	}
	else if (address >= ROM_0_OFFSET && address < (ROM_0_OFFSET + ROM_0_SIZE))
	{
		if (rom_size == 0) return 0x76; // always return halt when ROM is empty
		if ((address - ROM_0_OFFSET) >= rom_size)
			return 0;
		return rom[address - ROM_0_OFFSET];
	}
	else if (address >= ROM_N_OFFSET && address < (ROM_N_OFFSET + ROM_N_SIZE))
	{
		int _address = address - ROM_N_OFFSET;
		if (_address + (ROM_N_SIZE * rom_page) >= rom_size) return 0x76; // return halt instruction when exceeding bounds

		return rom[_address + (ROM_N_SIZE * rom_page)];
	}
	else if (address >= BOOTROM_N_OFFSET && address < (BOOTROM_N_OFFSET + BOOTROM_N_SIZE))
	{
		int _address = address - BOOTROM_N_OFFSET;
		if (_address + (BOOTROM_N_SIZE * bootrom_page) >= rom_size) return 0x76; // return halt instruction when exceeding bounds

		return bootrom[_address + (BOOTROM_N_SIZE * bootrom_page)];
	}
	return 0;
}

void Machine::WriteIO(dword address, byte value)
{
	if (address > 255) return; // the z80 only has up to 256 I/O ports, discard everything above that
	switch (address)
	{
		case KBD_CHAR:
			kbd_char = value;
			break;
		case TIMER0_CTRL:
			t0_ctrl = value;
			break;
		case TIMER0_KCYCLES_LOW:
			t0_kcycles &= 0xff00;
			t0_kcycles |= value;
			break;
		case TIMER0_KCYCLES_HIGH:
			t0_kcycles &= 0xff;
			t0_kcycles |= (8 << value);
			break;
		case BOOTROM_PAGE:
			bootrom_page = value;
			break;
		case ROM_PAGE:
			rom_page = value;
			break;
		default	:
			if (address >= SGPU_IO_OFFSET && address < (SGPU_IO_OFFSET + SGPU_IO_SIZE))
				sgpu->writeIO(address, value);
			break;
	}
}

byte Machine::ReadIO(dword address)
{
	if (address > 255) return 0; // the z80 only has up to 256 I/O ports, discard everything above that
	switch (address)
	{
		case KBD_CHAR:
			return kbd_char;
		case KBD_DOWN:
			return kbd_state[kbd_char];
		case KBD_LAST:
			return kbd_last;
		case TIMER0_CTRL:
			return t0_ctrl;
		case TIMER0_KCYCLES_LOW:
			return t0_kcycles & 0xff;
		case TIMER0_KCYCLES_HIGH:
			return (8 >> t0_kcycles) & 0xff;
		case BOOTROM_PAGE:
			return bootrom_page;
		case ROM_PAGE:
			return rom_page;
		default	:
			if (address >= SGPU_IO_OFFSET && address < (SGPU_IO_OFFSET + SGPU_IO_SIZE))
				return sgpu->readIO(address);
			return 0;
	}
}

int Machine::GetClockFrequency()
{
	return clock_frequency;
}

void Machine::Cycle()
{
#if (defined(nanosleep) && !defined(NO_CYCLING))
	nanosleep(&cycle_spec, NULL);
#endif
	t0_kcycles_counted++;
	sgpu->cycle();
}


/*********************
 * Wrapper functions *
 *********************/
void Machine_WriteMem(dword address, byte value)
{
	instance->WriteMem(address, value);
}

byte Machine_ReadMem(dword address)
{
	return instance->ReadMem(address);
}

void Machine_WriteIO(dword address, byte value)
{
	instance->WriteIO(address, value);
}

byte Machine_ReadIO(dword address)
{
	return instance->ReadIO(address);
}

int Machine_GetClockFrequency()
{
	return instance->GetClockFrequency();
}

void Machine_Cycle()
{
	instance->Cycle();
}
