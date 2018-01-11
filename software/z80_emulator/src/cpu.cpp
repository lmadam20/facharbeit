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


#include "cpu.h"
#include "wrappers.h"
#include <config.standard.h>

CPU::CPU()
{
	reset();
}

void CPU::reset()
{
	af.af = 0;
	bc.bc = 0;
	de.de = 0;
	hl.hl = 0;
	sp = RESET_SP;
	pc = RESET_PC;
	halted = 0;
	irq = 0;
	irq_disabled = 0;
	irq_processing = 0;
	irq_state_change_counter = 0;
	irq_change_state = 0xff;
}

void CPU::printState()
{
	cout << "----- Register dump -----" << endl;
	cout << hex;
	cout << "AF = " << af.af << endl;
	cout << "A = " << (dword)af.a << endl;
	cout << "BC = " << bc.bc << endl;
	cout << "DE = " << de.de << endl;
	cout << "HL = " << hl.hl << endl;
	cout << "SP = " << sp << endl;
 	cout << "PC = " << pc << endl;
	cout << "flags = " << (dword)af.f << endl << endl; // workaround

	cout << "Halted = " << halted << endl;
	cout << "IRQ = " << (dword)irq << endl; // ditto
	cout << "IRQ processing = " << (dword)irq_processing << endl;
	cout << "IRQ disabled = " << (dword)irq_disabled << endl << endl;

	cout << "----- Memory dump -----" << endl;
	cout << "Stack pointer" << endl;
	hexdump(sp, "SP", 1);
	hexdump(sp, "SP", 0);
	cout << endl << "Program counter" << endl;
	hexdump(pc, "PC", 0);
	cout << dec;
	cout << endl;
}

void CPU::hexdump(int addr, string label, int downwards)
{
	cout << hex;
	for (int i = 0; i < 4; i++)
	{
		cout << label;
		if (!downwards) cout << "+0x" << ((i + 1) * 8) - 1 << "\t";
		else cout << "-0x" << ((i + 1) * 8) - 1 << "\t";
		for (int j = 0; j < 8; j++)
		{
			if (!downwards) cout << "0x" << ((dword)Machine_ReadMem(addr + (i * 8 + j)) & 0xff) << " ";
			else cout << "0x" << ((dword)Machine_ReadMem(addr + (-i * 8 - j)) & 0xff) << " ";
		}
		cout << endl;
	}
	cout << dec;
}

void CPU::next()
{
	FOR_I_(4, cycle());
	if (halted) // CPU is halted
	{
		if (irq && !irq_disabled) // continue operation if IRQ has been fired and irq not disabled
		{
			halted = 0;
		}
		return;
	}

	if (irq && irq_processing && !irq_disabled)
	{
		irq_processing = 1;

		/* Push current PC to stack and jump to 0038h */
		push(pc);
		pc = 0x0038;
	}

	if (irq_state_change_counter == 0 && irq_change_state < 0xff)
	{
		if (irq_change_state == 1) irq_disabled = 0;
		else irq_disabled = 1;
		irq_change_state = 0xff;
	}
	else if (irq_state_change_counter != 0 && irq_change_state < 0xff)
	{
		irq_state_change_counter--;
	}

	byte opcode0 = Machine_ReadMem(pc++);
	if (opcode0 == 0xCB || opcode0 == 0xDD || opcode0 == 0xFD || opcode0 == 0xED)
	{
		byte opcode1 = Machine_ReadMem(pc++);
		if (opcode0 == 0xED)
		{
			/* RETI */
			if (opcode1 == 0x4D)
			{
				if (irq_processing)
				{
					FOR_I_(10, cycle());
					pc = pop();
					irq = 0;
					irq_processing = 0;
					irq_disabled = 1;
				}
			}

			/* DDS (Debug Dump State) */
			if (opcode1 == 0xFF)
			{
				cout << "DDS -> printState()" << endl;
				printState();
			}
		}
	}
	else if (opcode0 != 0x00) // != NOP
	{
		dword opcode1 = 0;
		/* Instruction operand fetching */
		switch (opcode0)
		{
			/* 2-byte operand */
			case 0x01:
			case 0x11:
			case 0x21:
			case 0x31:
			case 0xC2:
			case 0xD2:
			case 0xE2:
			case 0xF2:
			case 0xC3:
			case 0xC4:
			case 0xD4:
			case 0xE4:
			case 0xF4:
			case 0x2A:
			case 0x3A:
			case 0xCA:
			case 0xDA:
			case 0xEA:
			case 0xFA:
			case 0xCC:
			case 0xDC:
			case 0xEC:
			case 0xFC:
			case 0xCD:
			case 0x32:
				FOR_I_(6, cycle()); // all these instruction need at least 10 cycles, so 10 - 4 = 6
				opcode1 = Machine_ReadMem(pc++);
				opcode1 |= (Machine_ReadMem(pc++) << 8);
				break;

			/* 1-byte operand */
			case 0x10:
			case 0x20:
			case 0x30:
			case 0xD3:
			case 0x06:
			case 0x16:
			case 0x26:
			case 0x36:
			case 0xC6:
			case 0xD6:
			case 0xE6:
			case 0xF6:
			case 0x18:
			case 0x28:
			case 0x38:
			case 0xDB:
			case 0x0E:
			case 0x1E:
			case 0x2E:
			case 0x3E:
			case 0xCE:
			case 0xDE:
			case 0xEE:
			case 0xFE:
				FOR_I_(3, cycle());
				opcode1 = Machine_ReadMem(pc++);
				break;

			default:
				break;
		}

		/* LD r, r' */
		if (RANGE(opcode0, 0x40, 0x45) || RANGE(opcode0, 0x47, 0x4D) || RANGE(opcode0, 0x50, 0x55) || RANGE(opcode0, 0x57, 0x5D)
			|| RANGE(opcode0, 0x60, 0x65) || RANGE(opcode0, 0x67, 0x6D) || RANGE(opcode0, 0x78, 0x7D) || opcode0 == 0x4F
			|| opcode0 == 0x5F || opcode0 == 0x6F || opcode0 == 0x7F)
		{
			setRegisterValueByCode((opcode0 >> 3) & 0x7, getRegisterValueByCode(opcode0 & 0x7));
		}
		/* LD r, n */
		else if (opcode0 == 0x06 || opcode0 == 0x16 || opcode0 == 0x26 || opcode0 == 0x0E || opcode0 == 0x1E || opcode0 == 0x2E || opcode0 == 0x3E)
		{
			setRegisterValueByCode((opcode0 >> 3) & 0x7, opcode1);
		}
		/* LD r, (HL) */
		else if (opcode0 == 0x46 || opcode0 == 0x56 || opcode0 == 0x66 || opcode0 == 0x4E || opcode0 == 0x5E || opcode0 == 0x6E || opcode0 == 0x7E)
		{
			setRegisterValueByCode((opcode0 >> 3) & 0x7, Machine_ReadMem(hl.hl));
		}
		/* LD (HL), r */
		else if ((RANGE(opcode0, 0x70, 0x77) && opcode0 != 0x76) || opcode0 == 0x36 || opcode0 == 0x46 || opcode0 == 0x56 || opcode0 == 0x66)
		{
			Machine_WriteMem(hl.hl, getRegisterValueByCode(opcode0 & 0x07));
		}
		/* LD (HL), n */
		else if (opcode0 == 0x36)
		{
			Machine_WriteMem(hl.hl, opcode1 & 0xff);
		}
		/* LD A, (BC/DE/nn) */
		else if (opcode0 == 0x0A || opcode0 == 0x1A || opcode0 == 0x3A)
		{
			switch (opcode0)
			{
				case 0x0A:
					af.a = Machine_ReadMem(bc.bc);
					break;
				case 0x1A:
					af.a = Machine_ReadMem(de.de);
					break;
				case 0x3A:
					af.a = Machine_ReadMem(opcode1);
					break;
			}
		}
		/* LD (BC/DE/nn), A */
		else if (opcode0 == 0x02 || opcode0 == 0x12 || opcode0 == 0x32)
		{
			switch (opcode0)
			{
				case 0x02:
					Machine_WriteMem(bc.bc, af.a);
					break;
				case 0x12:
					Machine_WriteMem(de.de, af.a);
					break;
				case 0x32:
					Machine_WriteMem(opcode1, af.a);
					break;
			}
		}
		/* LD dd, nn */
		else if (opcode0 == 0x01 || opcode0 == 0x11 || opcode0 == 0x21 || opcode0 == 0x31)
		{
			setRegisterPairValueByCode((opcode0 >> 4) & 0x3, opcode1);
		}
		/* LD HL, (nn) */
		else if (opcode0 == 0x2A)
		{
			FOR_I_(6, cycle());
			hl.h = Machine_ReadMem(opcode1 + 1);
			hl.l = Machine_ReadMem(opcode1);
		}
		/* LD (nn), HL */
		else if (opcode0 == 0x22)
		{
			FOR_I_(6, cycle());
			Machine_WriteMem(opcode1 + 1, hl.h);
			Machine_WriteMem(opcode1, hl.l);
		}
		/* LD SP, HL */
		else if (opcode0 == 0xF9)
		{
			FOR_I_(2, cycle());
			sp = hl.hl;
		}
		/* PUSH qq */
		else if (opcode0 == 0xC5 || opcode0 == 0xD5 || opcode0 == 0xE5 || opcode0 == 0xF5)
		{
			FOR_I_(7, cycle());
			unsigned int pair;
			if (((opcode0 >> 4) & 0x3) != 3) pair = getRegisterPairValueByCode((opcode0 >> 4) & 0x3);
			else pair = af.af;
			push(pair);
		}
		/* POP qq */
		else if (opcode0 == 0xC1 || opcode0 == 0xD1 || opcode0 == 0xE1 || opcode0 == 0xF1)
		{
			FOR_I_(6, cycle());
			int value = pop();
			if (((opcode0 >> 4) & 0x3) != 3) setRegisterPairValueByCode((opcode0 >> 4) & 0x3, value);
			else af.af = value;
		}
		/* ADD A, r */
		else if (RANGE(opcode0, 0x80, 0x87) && opcode0 != 0x86)
		{
			unsigned int old = (unsigned int)af.a;
			af.a += getRegisterValueByCode(opcode0 & 0x3);
			if (af.a < old) SET_BIT(af.f, FLAG_CARRY_POS);
			updateFlags(af.a);
		}
		/* ADD A, n */
		else if (opcode0 == 0xC6)
		{
			unsigned int old = (unsigned int)af.a;
			af.a += opcode1 & 0xff;
			if (af.a < old) SET_BIT(af.f, FLAG_CARRY_POS);
			updateFlags(af.a);
		}
		/* ADD A, (HL) */
		else if (opcode0 == 0x86)
		{
			unsigned int old = (unsigned int)af.a;
			FOR_I_(3, cycle());
			af.a += Machine_ReadMem(hl.hl);
			if (af.a < old) SET_BIT(af.f, FLAG_CARRY_POS);
			updateFlags(af.a);
		}
		/* SUB A, r */
		else if (RANGE(opcode0, 0x90, 0x97) && opcode0 != 0x96)
		{
			af.a -= getRegisterValueByCode(opcode0 & 0x3);
			updateFlags(af.a);
		}
		/* SUB A, n */
		else if (opcode0 == 0xD6)
		{
			af.a -= opcode1 & 0xff;
			updateFlags(af.a);
		}
		/* SUB A, (HL) */
		else if (opcode0 == 0x96)
		{
			FOR_I_(3, cycle());
			af.a -= Machine_ReadMem(hl.hl);
			updateFlags(af.a);
		}
		/* AND A, r */
		else if (RANGE(opcode0, 0xA0, 0xA7) && opcode0 != 0xA6)
		{
			af.a &= getRegisterValueByCode(opcode0 & 0x3);
			updateFlags2(af.a);
		}
		/* AND A, n */
		else if (opcode0 == 0xE6)
		{
			af.a &= opcode1 & 0xff;
			updateFlags2(af.a);
		}
		/* AND A, (HL) */
		else if (opcode0 == 0xA6)
		{
			FOR_I_(3, cycle());
			af.a &= Machine_ReadMem(hl.hl);
			updateFlags2(af.a);
		}
		/* OR A, r */
		else if (RANGE(opcode0, 0xB0, 0xB7) && opcode0 != 0xB6)
		{
			af.a |= getRegisterValueByCode(opcode0 & 0x3);
			updateFlags2(af.a);
		}
		/* OR A, n */
		else if (opcode0 == 0xF6)
		{
			af.a |= opcode1 & 0xff;
			updateFlags2(af.a);
		}
		/* OR A, (HL) */
		else if (opcode0 == 0xB6)
		{
			FOR_I_(3, cycle());
			af.a |= Machine_ReadMem(hl.hl);
			updateFlags2(af.a);
		}
		/* XOR A, r */
		else if (RANGE(opcode0, 0xA8, 0xAF) && opcode0 != 0xAE)
		{
			af.a ^= getRegisterValueByCode(opcode0 & 0x3);
			updateFlags2(af.a);
		}
		/* XOR A, n */
		else if (opcode0 == 0xEE)
		{
			af.a ^= opcode1 & 0xff;
			updateFlags2(af.a);
		}
		/* XOR A, (HL) */
		else if (opcode0 == 0xAE)
		{
			FOR_I_(3, cycle());
			af.a ^= Machine_ReadMem(hl.hl);
			updateFlags2(af.a);
		}
		/* JP nn */
		else if (opcode0 == 0xC3)
		{
			pc = opcode1;
		}
		/* JP cc, nn */
		else if (opcode0 == 0xC2 || opcode0 == 0xD2 || opcode0 == 0xE2 || opcode0 == 0xF2 ||
			opcode0 == 0xCA || opcode0 == 0xDA || opcode0 == 0xEA || opcode0 == 0xFA)
		{
			if (isConditionTrue((opcode0 >> 3) & 0x3)) pc = opcode1;
		}
		/* JP (HL) */
		else if (opcode0 == 0xE9)
		{
			pc = hl.hl;
		}
		/* CALL nn */
		else if (opcode0 == 0xCD)
		{
			FOR_I_(7, cycle());
			push(pc);
			pc = opcode1;
		}
		/* CALL cc, nn */
		else if (opcode0 == 0xC4 || opcode0 == 0xD4 || opcode0 == 0xE4 || opcode0 == 0xF4 ||
			opcode0 == 0xCC || opcode0 == 0xDC || opcode0 == 0xEC || opcode0 == 0xFC)
		{
			if (isConditionTrue((opcode0 >> 3) & 0x3))
			{
				FOR_I_(7, cycle());
				push(pc + 2);
				pc = opcode1;
			}
		}
		/* RET */
		else if (opcode0 == 0xC9)
		{
			pc = pop();
			FOR_I_(6, cycle());
		}
		/* RET cc */
		else if (opcode0 == 0xC0 || opcode0 == 0xD0 || opcode0 == 0xE0 || opcode0 == 0xF0 ||
			opcode0 == 0xC8 || opcode0 == 0xD8 || opcode0 == 0xE8 || opcode0 == 0xF8)
		{
			cycle();
			if (isConditionTrue((opcode0 >> 3) & 0x3))
			{
				FOR_I_(6, cycle());
				pc = pop();
			}
		}
		/* IN A, (n) */
		else if (opcode0 == 0xDB)
		{
			FOR_I_(4, cycle());
			af.a = Machine_ReadIO(opcode1 & 0xff);
		}
		/* OUT (n), A */
		else if (opcode0 == 0xD3)
		{
			FOR_I_(4, cycle());
			Machine_WriteIO(opcode1 & 0xff, af.a);
		}
		/* INC ss */
		else if (opcode0 == 0x03 || opcode0 == 0x13 || opcode0 == 0x23 || opcode0 == 0x33)
		{
			FOR_I_(2, cycle());
			int mask = (opcode0 >> 4) & 0x3;
			int value = getRegisterPairValueByCode(mask);
			setRegisterPairValueByCode(mask, value + 1);
		}
		/* DEC ss */
		else if (opcode0 == 0x0B || opcode0 == 0x1B || opcode0 == 0x2B || opcode0 == 0x3B)
		{
			FOR_I_(2, cycle());
			int mask = (opcode0 >> 4) & 0x3;
			int value = getRegisterPairValueByCode(mask);
			setRegisterPairValueByCode(mask, value - 1);
		}
		/* RLCA */
		else if (opcode0 == 0x07)
		{
			int bit7 = GET_BIT(af.a, 7);
			bit7 != 0 ? bit7 = 1 : bit7 = 0;
			if (bit7 == 1) { SET_BIT(af.a, 7); SET_BIT(af.f, FLAG_CARRY_POS); }
			else { CLR_BIT(af.a, 7); CLR_BIT(af.f, FLAG_CARRY_POS); }
			CLR_BIT(af.f, FLAG_HALFCARRY_POS);
			CLR_BIT(af.f, FLAG_ADDSUBTRACT_POS);
		}
		/* RLA */
		else if (opcode0 == 0x17)
		{
			int bit7 = GET_BIT(af.a, 7);
			bit7 != 0 ? bit7 = 1 : bit7 = 0;
			if (bit7 == 1) SET_BIT(af.f, FLAG_CARRY_POS);
			else CLR_BIT(af.f, FLAG_CARRY_POS);
			CLR_BIT(af.f, FLAG_HALFCARRY_POS);
			CLR_BIT(af.f, FLAG_ADDSUBTRACT_POS);
		}
		/* RRCA */
		else if (opcode0 == 0x0F)
		{
			int bit0 = GET_BIT(af.a, 0);
			af.a >>= 1;
			bit0 &= 1;
			if (bit0 == 1) { SET_BIT(af.a, 7); SET_BIT(af.f, FLAG_CARRY_POS); }
			else { CLR_BIT(af.a, 7); CLR_BIT(af.f, FLAG_CARRY_POS); }
			CLR_BIT(af.f, FLAG_HALFCARRY_POS);
			CLR_BIT(af.f, FLAG_ADDSUBTRACT_POS);
		}
		/* RRA */
		else if (opcode0 == 0x1F)
		{
			int bit0 = GET_BIT(af.a, 0);
			af.a >>= 1;
			bit0 &= 1;
			if (bit0 == 1) SET_BIT(af.f, FLAG_CARRY_POS);
			else CLR_BIT(af.f, FLAG_CARRY_POS);
			CLR_BIT(af.f, FLAG_HALFCARRY_POS);
			CLR_BIT(af.f, FLAG_ADDSUBTRACT_POS);
		}
		/* EI */
		else if (opcode0 == 0xFB)
		{
			irq_change_state = 1;
			irq_state_change_counter = 2;
		}
		/* DI */
		else if (opcode0 == 0xF3)
		{
			irq_change_state = 0;
			irq_state_change_counter = 2;
		}
		/* RST p */
		else if ((opcode0 & 0xC7) == 0xC7)
		{
			int t = (opcode0 >> 3) & 0x7;
			pc = (t * 8);
			cout << "RST " << hex << (t * 8) << endl;
		}
		else
		{
			switch (opcode0)
			{
				case 0x76: // halt
					cout << "CPU has been halted until next interrupt or reset!" << endl;
					halted = 1;
					break;
				default:
					cout << "Illegal opcode @ " << hex << (pc - 1) << endl;
					break;
			}
		}
	}
}

void CPU::triggerIRQ()
{
	irq = 1;
}

void CPU::push(dword value)
{
	Machine_WriteMem(--sp, (value >> 8) & 0xff);
	Machine_WriteMem(--sp, value & 0xff);
}

dword CPU::pop()
{
	unsigned int value = Machine_ReadMem(sp++);
	value |= (Machine_ReadMem(sp++)) << 8;
	return value & 0xffff;
}

void CPU::cycle()
{
	Machine_Cycle();
}

void CPU::updateFlags(int registerValue)
{
	af.f = 0;
	(int8_t)registerValue < 0 ? SET_BIT(af.f, FLAG_SIGN_POS) : CLR_BIT(af.f, FLAG_SIGN_POS);
	registerValue == 0 ? SET_BIT(af.f, FLAG_ZERO_POS) : CLR_BIT(af.f, FLAG_ZERO_POS);
	CLR_BIT(af.f, FLAG_ADDSUBTRACT_POS);
}

void CPU::updateFlags2(int registerValue)
{
	af.f = 0;
	(int8_t)registerValue < 0 ? SET_BIT(af.f, FLAG_SIGN_POS) : CLR_BIT(af.f, FLAG_SIGN_POS);
	registerValue == 0 ? SET_BIT(af.f, FLAG_ZERO_POS) : CLR_BIT(af.f, FLAG_ZERO_POS);
	SET_BIT(af.f, FLAG_HALFCARRY_POS);
	CLR_BIT(af.f, FLAG_PARITYOVERFLOW_POS);
	CLR_BIT(af.f, FLAG_ADDSUBTRACT_POS);
	CLR_BIT(af.f, FLAG_CARRY_POS);
}

void CPU::setRegisterValueByCode(int code, int value)
{
	value &= 0xff; // make sure our value is <= 0xff
	switch (code)
	{
		case REG_CODE_A:
			af.a = value;
			break;
		case REG_CODE_B:
			bc.b = value;
			break;
		case REG_CODE_C:
			bc.c = value;
			break;
		case REG_CODE_D:
			de.d = value;
			break;
		case REG_CODE_E:
			de.e = value;
			break;
		case REG_CODE_H:
			hl.h = value;
			break;
		case REG_CODE_L:
			hl.l = value;
			break;
		default:
			return;
	}
}

int CPU::getRegisterValueByCode(int code)
{
	switch (code)
	{
		case REG_CODE_A:
			return af.a;
		case REG_CODE_B:
			return bc.b;
		case REG_CODE_C:
			return bc.c;
		case REG_CODE_D:
			return de.d;
		case REG_CODE_E:
			return de.e;
		case REG_CODE_H:
			return hl.h;
		case REG_CODE_L:
			return hl.l;
		default:
			return 0;
	}
}

void CPU::setRegisterPairValueByCode(int code, int value)
{
	value &= 0xffff;
	switch (code)
	{
		case REG_CODE_BC:
			bc.bc = value;
			break;
		case REG_CODE_DE:
			de.de = value;
			break;
		case REG_CODE_HL:
			hl.hl = value;
			break;
		case REG_CODE_SP:
			sp = value;
			break;
		default:
			return;
	}
}

int CPU::getRegisterPairValueByCode(int code)
{
	switch (code)
	{
		case REG_CODE_BC:
			return bc.bc;
		case REG_CODE_DE:
			return de.de;
		case REG_CODE_HL:
			return hl.hl;
		case REG_CODE_SP:
			return sp;
		default:
			return 0;
	}
}

bool CPU::isConditionTrue(int code)
{
	switch (code)
	{
		case 0: // NZ
			return !GET_BIT(af.f, FLAG_ZERO_POS);
		case 1: // Z
			return GET_BIT(af.f, FLAG_ZERO_POS);
		case 2: // NC
			return !GET_BIT(af.f, FLAG_CARRY_POS);
		case 3: // C
			return GET_BIT(af.f, FLAG_CARRY_POS);
		case 4: // O
			return !GET_BIT(af.f, FLAG_PARITYOVERFLOW_POS);
		case 5: // E
			return GET_BIT(af.f, FLAG_PARITYOVERFLOW_POS);
		case 6: // P
			return GET_BIT(af.f, FLAG_SIGN_POS);
		case 7: // N
			return !GET_BIT(af.f, FLAG_SIGN_POS);
		default:
			return false;
	}
}

CPU::~CPU()
{
	printState();
}
