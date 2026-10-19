/* A work-in-progess MEGA65 (Commodore-65 clone origins) emulator
   Part of the Xemu project, please visit: https://github.com/lgblgblgb/xemu
   Copyright (C)2016-2025 LGB (Gábor Lénárt) <lgblgblgb@gmail.com>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA */

#ifndef XEMU_MEGA65_RECORDER_H_INCLUDED
#define XEMU_MEGA65_RECORDER_H_INCLUDED

// Video frames: the visible area (as the window shows it without full
// borders), 24 bit RGB, one frame per emulated frame. NTSC frames are padded
// with black at the bottom to the PAL height.
#define RECORDER_X	48
#define RECORDER_WIDTH	704
#define RECORDER_HEIGHT	576

extern void recorder_init  ( const char *video_fn, const char *audio_fn );
extern void recorder_frame ( const Uint32 *pixels, const int texture_width, const int visible_height, const int frame_usec );

#endif
