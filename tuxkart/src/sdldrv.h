//  $Id: sdldrv.h,v 1.6 2004/08/05 10:19:49 jamesgregory Exp $
//
//  TuxKart - a fun racing game with go-kart
//  Copyright (C) 2004 James Gregory <james.gregory@btinternet.com>
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

#ifndef HEADER_SDLDRV_H
#define HEADER_SDLDRV_H

#include <SDL.h>

extern Uint8 *keyState;
extern SDL_Surface *sdl_screen;

void initVideo (int w, int h, bool fullscreen);
void shutdownVideo();
void pollEvents();
void kartInput();
void keyboardInput (const SDL_keysym& key);
void swapBuffers();
int  getScreenWidth();
int  getScreenHeight();

#endif

/* EOF */
