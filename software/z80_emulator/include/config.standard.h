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


/* All sizes/offsets in bytes */

/* RAM */
#define RAM_SIZE 0x3000
#define RAM_OFFSET 0x1000

/* Bootrom page 0 */
#define BOOTROM_0_SIZE 0x2000
#define BOOTROM_0_OFFSET ((0xFFFF - 0x2000) + 1)

/* Bootrom page n */
#define BOOTROM_N_SIZE 0x2000
#define BOOTROM_N_OFFSET (BOOTROM_0_OFFSET - BOOTROM_N_SIZE) + 1

/* ROM page 0 */
#define ROM_0_SIZE 0x2000
#define ROM_0_OFFSET 0x4000

/* ROM page n */
#define ROM_N_SIZE 0x2000
#define ROM_N_OFFSET (ROM_0_OFFSET + ROM_0_SIZE)

/* Framebuffer page n */
#define FB_N_SIZE (FB_WIDTH * FB_LINE_HEIGHT)
#define FB_N_OFFSET (ROM_N_OFFSET + ROM_N_SIZE)

/* Framebuffer size */
#define FB_WIDTH 160
#define FB_HEIGHT 120
#define FB_LINE_HEIGHT 10

/* IO ports */

#define SGPU_IO_OFFSET 		10
#define SGPU_IO_SIZE 		10
#define SGPU_FB_PAGE_NUMBER 0
#define _SGPU_CMD_BUF_ADDR 	1
#define _SGPU_CMD_BUF_VALUE	2

#define SGPU_CMD_FILL		0x01
#define SGPU_CMD_TTY_WRITE	0x02
#define SGPU_CMD_ACK		0xff
#define SGPU_CMD_NACK		0xfe

#define KBD_CHAR 0			// keyboard character
#define KBD_DOWN 1
#define KBD_LAST 2			// latest key pressed (0 = currently no key pressed)
#define FB_PAGE_NUMBER (SGPU_IO_OFFSET + SGPU_FB_PAGE_NUMBER)
#define SGPU_CMD_BUF_ADDR 	(SGPU_IO_OFFSET + _SGPU_CMD_BUF_ADDR)
#define SGPU_CMD_BUF_VALUE 	(SGPU_IO_OFFSET + _SGPU_CMD_BUF_VALUE)
#define TIMER0_CTRL	20			// control register for timer 0
#define TIMER0_KCYCLES_LOW 21	// number of kilo cycles to count until IRQ or set ellapsed bit (low byte)
#define TIMER0_KCYCLES_HIGH 22	// same as above, but high byte

#define BOOTROM_PAGE	30
#define ROM_PAGE		40

/* SGPU */
#define TTY_HEIGHT 	12
#define TTY_WIDTH 	16

/* Filenames */
#define BOOTROM_FILENAME "bootrom.bin"

/* Default vectors */
#define RESET_PC BOOTROM_0_OFFSET
#define RESET_SP 0x00FF

/* Clocking */
#define CLOCK_FREQUENCY 1000000 // in Hz

/* Timer 0 */

// Bit positions for control register
#define T0_CTRL_ENABLE 0
#define T0_CTRL_ENABLE_IRQ 1
#define T0_CTRL_TRIGGER 2
// others are unused

#define FONT_DEFAULT_NAME "Fyodor-Bold.ttf"

#define WINDOW_TITLE "Z80-like Emulator"

