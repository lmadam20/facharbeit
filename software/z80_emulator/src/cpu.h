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

#include <stdafx.h>

#define REG_CODE_A 7
#define REG_CODE_B 0
#define REG_CODE_C 1
#define REG_CODE_D 2
#define REG_CODE_E 3
#define REG_CODE_H 4
#define REG_CODE_L 5

#define REG_CODE_BC 0
#define REG_CODE_DE 1
#define REG_CODE_HL 2
#define REG_CODE_SP 3

#define FLAG_SIGN_POS 7
#define FLAG_ZERO_POS 6
#define FLAG_HALFCARRY_POS 4
#define FLAG_PARITYOVERFLOW_POS 2
#define FLAG_ADDSUBTRACT_POS 1
#define FLAG_CARRY_POS 0

class CPU
{
public:
	CPU();

	void printState();

	void reset();

	/**
     * Lets the CPU process the next instruction
     */
	void next();

	/**
     * Cycle for one clock period (1/f)
     */
	void cycle();

	/**
	 * Request maskable interrupt (IRQ)
	 */
	void triggerIRQ();

	~CPU();

private:
	void push(dword value);

	dword pop();

	void setRegisterValueByCode(int code, int value);

	int getRegisterValueByCode(int code);

	void setRegisterPairValueByCode(int code, int value);

	int getRegisterPairValueByCode(int code);

	bool isConditionTrue(int code);

	void updateFlags(int registerValue);

	void updateFlags2(int registerValue);

	void hexdump(int addr, string label, int downwards);

	/* Register set */
	union
	{
		dword af;
		struct
		{
#if defined(BIG_ENDIAN) || defined(MSVC)
			byte f, a;
#else
			byte a, f;
#endif
		};
	} af;

	union
	{
		dword bc;
		struct
		{
#if defined(BIG_ENDIAN) || defined(MSVC)
			byte c, b;
#else
			byte b, c;
#endif
		};
	} bc;

	union
	{
		dword de;
		struct
		{
#if defined(BIG_ENDIAN) || defined(MSVC)
			byte e, d;
#else
			byte d, e;
#endif
		};
	} de;

	union
	{
		dword hl;
		struct
		{
#if defined(BIG_ENDIAN) || defined(MSVC)
			byte l, h;
#else
			byte h, l;
#endif
		};
	} hl;

	dword pc;
	dword sp;
	byte irq;
	byte irq_processing;
	byte irq_state_change_counter;
	byte irq_change_state;
	byte irq_disabled;
	int halted;
};
