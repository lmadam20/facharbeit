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

#include <iostream>
#include <fstream>
#include <string>
#include <algorithm> // min
#include <cstring> // memcpy
#include <time.h> // nanosleep
#include <stdint.h>
#include <ctime>

using namespace std;

typedef uint8_t byte;
typedef unsigned short dword;


/*** Macros ***/

/* Bits */
#define GET_BIT(x, pos) ((x >> pos) & 1)
#define SET_BIT(x, pos) (x |= (1 << pos))
#define CLR_BIT(x, pos) (x &= ~(1 << pos))

/* Loops */
#define FOR_I(x) for (int j = 0; j < x; j++)
#define FOR_I_(x, code) FOR_I(x) { code; }

/* Conditions */
#define RANGE(x, from, to) (x >= from && x <= to)

