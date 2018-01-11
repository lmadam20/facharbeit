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
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

class SGPU
{
public:
	/**
	 * Creates a GPU and maps it to
	 * the address 'addr'
	 */
	SGPU(int addr);

	int init(int width, int height);

	int initFB0(int width, int height);

	void writeFB(dword addr, byte value);

	byte readFB(dword addr);

	void writeIO(byte port, byte value);

	byte readIO(byte port);

	byte* getFB0();

	int render();

	void cycle();

	/**
	 * Prints out debugging information.
	 */
	void dump();

	~SGPU();

private:
	int fillBlock();

	int writeCharacter();

	void setCmdBufId(int id);

	int getCmdBufId();

	void stopCommand(int status, int response);

	int addr;
	byte* framebuffer0;
	int fb0_width, fb0_height;
	byte framebuffer_page;
	bool framebuffer_changed;

	/* Command buffer */
	struct
	{
		byte data[256];
		bool trigger;
	} cmd_buf;
	byte cmd_buf_addr;
	unsigned int cmd_tmp;

	/* Internal stuff */
	SDL_TimerID framebuffer_timer;
	SDL_Window* window;
	SDL_Renderer* renderer;
	SDL_Texture* fb0_texture;

	/* Console rendering */
	TTF_Font* default_font;
	SDL_Texture* tty_tex;
	char* tty_buffer;
	int tty_index, tty_size;
	int tty_changed;
	int cursor_x, cursor_y;
};
