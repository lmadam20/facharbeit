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

#include "sgpu.h"
#include "endian.h"
#include <config.standard.h>

SGPU::SGPU(int addr)
{
	this->addr = addr;
	framebuffer_page = 0;
	framebuffer_changed = false;
}

int SGPU::init(int width, int height)
{
	/* Window initialization */
	window = SDL_CreateWindow(WINDOW_TITLE, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, 0);
	if (!window)
	{
		cerr << "Failed to create window: " << SDL_GetError() << endl;
		return -1;
	}

	/* Framebuffer stuff */
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	fb0_texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB332, SDL_TEXTUREACCESS_STREAMING, FB_WIDTH, FB_HEIGHT);

	/* Command buffer initalization */
	memset(&cmd_buf, 0, sizeof(cmd_buf));

	/* Console (tty) initialization */
	if (TTF_Init() == -1)
	{
		cout << "Failed to initialize TTF library. " << SDL_GetError() << endl;
	}

	cout << "Loading " << FONT_DEFAULT_NAME << " as default font..." << endl;
	default_font = TTF_OpenFont(FONT_DEFAULT_NAME, 30);
	if (!default_font)
	{
		cerr << "Failed to load default font: " << SDL_GetError() << endl;
		return -1;
	}

	tty_index = 0;
	tty_size = TTY_WIDTH * TTY_HEIGHT;
	tty_changed = false;
	tty_buffer = new char[tty_size + 1];
	if (!tty_buffer)
	{
		cerr << "Failed to allocate buffer for console" << endl;
		return -1;
	}
	memset(tty_buffer, ' ', tty_size * sizeof(char));
	tty_buffer[tty_size] = 0;


	return 0;
}

int SGPU::initFB0(int width, int height)
{
	framebuffer0 = new byte[width * height];
	fb0_width = width;
	fb0_height = height;
	memset(framebuffer0, 0, width * height);
	return 1;
}

void SGPU::writeFB(dword page_addr, byte value)
{
	int fb_addr = (framebuffer_page * FB_N_SIZE + page_addr) - addr;
	framebuffer_changed = true;
	if (fb_addr <= (FB_WIDTH * FB_HEIGHT))
	{
		framebuffer0[fb_addr] = value;
	}
}

byte SGPU::readFB(dword page_addr)
{
	int fb_addr = (framebuffer_page * FB_N_SIZE + page_addr) - addr;
	if (fb_addr <= (FB_WIDTH * FB_HEIGHT))
	{
		return framebuffer0[fb_addr];
	}
	return 0;
}

void SGPU::writeIO(byte port, byte value)
{
	port -= SGPU_IO_OFFSET;
	switch (port)
	{
		case SGPU_FB_PAGE_NUMBER:
			framebuffer_page = value;
			break;
		case _SGPU_CMD_BUF_ADDR:
			cmd_buf_addr = value;
			break;
		case _SGPU_CMD_BUF_VALUE:
			cmd_buf.data[cmd_buf_addr & 0xff] = value & 0xff;
			break;
		default:
			break;
	}
}

byte SGPU::readIO(byte port)
{
	port -= SGPU_IO_OFFSET;
	switch (port)
	{
		case SGPU_FB_PAGE_NUMBER:
			return framebuffer_page & 0xff;
		case _SGPU_CMD_BUF_ADDR:
			return cmd_buf_addr;
		case _SGPU_CMD_BUF_VALUE:
			return cmd_buf.data[cmd_buf_addr & 0xff];
		default:
			break;
	}
	return 0;
}

byte* SGPU::getFB0()
{
	return framebuffer0;
}

int SGPU::render()
{
	if (framebuffer_changed)
	{
		SDL_UpdateTexture(fb0_texture, NULL, framebuffer0, FB_WIDTH);
		SDL_RenderCopy(renderer, fb0_texture, NULL, NULL);
		framebuffer_changed = false;
	}
	if (tty_changed)
	{
		SDL_Rect tty_rect;
		tty_rect.x = 0;
		tty_rect.y = 0;
		tty_rect.w = FB_WIDTH;
		tty_rect.h = FB_HEIGHT;

		SDL_RenderCopy(renderer, tty_tex, NULL, &tty_rect); // Copy TTY on to Framebuffer 1
		tty_changed = false;
	}

	SDL_RenderPresent(renderer);
	return 0;
}

void SGPU::cycle()
{
	int id = getCmdBufId();
	int status = (int)cmd_buf.data[0];
	if (GET_BIT(status, 0)) // Bit 0 of status byte is a trigger bit
	{
		if (!cmd_buf.trigger)
		{
			cmd_buf.trigger = true;
			cmd_tmp = 0;
		}
		if (cmd_buf.trigger)
		{
			switch (id)
			{
				case SGPU_CMD_FILL:
					if (!fillBlock())
					{
						stopCommand(status, SGPU_CMD_ACK);
					}
					break;
				case SGPU_CMD_TTY_WRITE:
					if (!default_font)
						stopCommand(status, SGPU_CMD_NACK);
					if (!writeCharacter())
						stopCommand(status, SGPU_CMD_ACK);
					break;
				default:
					stopCommand(status, SGPU_CMD_NACK);
					break;
			}
		}
	}
}

void SGPU::dump()
{
	cout << "========= SGPU command buffer dump =========" << endl;
	cout << "Buffer hexdump:" << endl;
	for (int i = 0; i < 16; i++)
	{
		for (int j = 0; j < 16; j++)
			cout << hex << (dword)cmd_buf.data[(i * 16) + j] << " ";
		cout << endl;
	}

	cout << endl << "command id = " << getCmdBufId() << endl;
	cout << "trigger = " << (cmd_buf.trigger ? "True" : "False") << endl;
	cout << "============================================" << endl << endl;
}

SGPU::~SGPU()
{
	dump();
	delete[] framebuffer0;
}

int SGPU::fillBlock()
{
	if (cmd_buf.data[2] == 0)
	{
		unsigned int addr = (unsigned int)((cmd_buf.data[3] << 8) | cmd_buf.data[4]);
		unsigned int count = (unsigned int)((cmd_buf.data[5] << 8) | cmd_buf.data[6]);

#ifndef LITTLE_ENDIAN
		addr = endian_swap(addr);
		count = endian_swap(count);
#endif // LITTLE_ENDIAN


		if ((cmd_tmp + addr) >= (unsigned int)(fb0_width * fb0_height + 1))
		{
			cerr << "count = " << hex << count << endl;
			cerr << "Illegal access to Framebuffer (index exceeds size)" << endl;
			return 0;
		}

		int i;
		for (i = 0; i < 8; i++)
		{
			if (cmd_tmp >= count)
			{
				return 0;
			}

			framebuffer0[addr + cmd_tmp] = cmd_buf.data[7];
			cmd_tmp++;
		}
	}

	framebuffer_changed = true;

	return 1;
}

int SGPU::writeCharacter()
{
	char c = cmd_buf.data[2];
	int c_idx = 1;
	tty_index = cursor_y * TTY_WIDTH + cursor_x;
	int count = 0;

	while (c != 0 && count <= tty_size)
	{
		if (tty_index >= tty_size) tty_index = 0; // wrap around when buffer is full

		tty_buffer[tty_index++] = c;
		c = cmd_buf.data[2 + c_idx++];
		count++;
	}
	if (tty_index >= tty_size) tty_index = tty_size - 1;
	tty_buffer[tty_index] = 0;

	SDL_Color White = {255, 255, 255};
	SDL_Surface* tty_surface = TTF_RenderText_Solid(default_font, tty_buffer, White);
	tty_tex = SDL_CreateTextureFromSurface(renderer, tty_surface);

	tty_changed = true;

	return 0;
}

void SGPU::setCmdBufId(int id)
{
	cmd_buf.data[1] = (id & 0xff);
}

int SGPU::getCmdBufId()
{
	return (int)cmd_buf.data[1];
}

void SGPU::stopCommand(int status, int response)
{
	cmd_tmp = 0;
	cmd_buf.trigger = false;
	setCmdBufId(response);
	CLR_BIT(status, 0);
	cmd_buf.data[0] = status;	
}
