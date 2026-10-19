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

/* Recording of the emulated machine's picture and sound (-recvideo, -recaudio),
   for making videos with an external encoder such as ffmpeg:

   - video: raw RGB24 frames of RECORDER_WIDTH x RECORDER_HEIGHT pixels, one per
     emulated frame (50 per second in PAL, 60 in NTSC)
   - audio: raw signed 16 bit stereo samples (native byte order) at
     AUDIO_SAMPLE_FREQ Hz

   The sound is rendered here, frame by frame, as much as the emulated frame
   lasted, not by the SDL audio callback. So picture and sound stay in step
   whatever speed the emulation runs at (also with -sleepless and -headless).
   Either file may be a named pipe. */

#include "xemu/emutools.h"
#include "mega65.h"
#include "audio65.h"
#include "recorder.h"

#include <signal.h>


static FILE *video_fp = NULL;
static FILE *audio_fp = NULL;
static Uint8 *line_buffer = NULL;
static Uint32 sample_remainder = 0;	// in 1/1000000 samples


static FILE *open_output ( const char *fn, const char *what )
{
	if (!fn || !*fn)
		return NULL;
	FILE *fp = fopen(fn, "wb");
	if (!fp)
		FATAL("Cannot open %s recording output: %s", what, fn);
	DEBUGPRINT("RECORDER: %s goes to %s" NL, what, fn);
	return fp;
}


void recorder_init ( const char *video_fn, const char *audio_fn )
{
	video_fp = open_output(video_fn, "video");
	audio_fp = open_output(audio_fn, "audio");
	if (video_fp) {
		line_buffer = xemu_malloc(RECORDER_WIDTH * 3);
		DEBUGPRINT("RECORDER: video is %dx%d RGB24" NL, RECORDER_WIDTH, RECORDER_HEIGHT);
	}
	if (audio_fp) {
		audio65_recording = true;
		DEBUGPRINT("RECORDER: audio is %d Hz, 16 bit signed stereo" NL, AUDIO_SAMPLE_FREQ);
	}
#ifdef SIGPIPE
	// When the reader of a pipe goes away, end the emulation properly
	// (see stop()) instead of being killed by the signal.
	if (video_fp || audio_fp)
		signal(SIGPIPE, SIG_IGN);
#endif
}


static void stop ( const char *what )
{
	DEBUGPRINT("RECORDER: cannot write %s any more, stopping the emulation" NL, what);
	// No more recording, also not of the frames emulated while shutting down
	video_fp = NULL;
	audio_fp = NULL;
	XEMUEXIT(0);
}


static void record_video ( const Uint32 *pixels, const int texture_width, const int visible_height )
{
	const SDL_PixelFormat *f = sdl_pix_fmt;
	for (int y = 0; y < RECORDER_HEIGHT; y++) {
		Uint8 *o = line_buffer;
		if (y < visible_height) {
			const Uint32 *p = pixels + y * texture_width + RECORDER_X;
			for (int x = 0; x < RECORDER_WIDTH; x++, p++) {
				*o++ = (*p & f->Rmask) >> f->Rshift;
				*o++ = (*p & f->Gmask) >> f->Gshift;
				*o++ = (*p & f->Bmask) >> f->Bshift;
			}
		} else
			memset(o, 0, RECORDER_WIDTH * 3);
		if (fwrite(line_buffer, RECORDER_WIDTH * 3, 1, video_fp) != 1)
			stop("video");
	}
}


static void record_audio ( const int frame_usec )
{
	static Sint16 buffer[2 * 4096];
	sample_remainder += (Uint32)frame_usec * AUDIO_SAMPLE_FREQ;
	int samples = sample_remainder / 1000000;
	sample_remainder %= 1000000;
	while (samples > 0) {
		const int n = samples > 4096 ? 4096 : samples;
		audio65_render_mix(buffer, n);
		if (fwrite(buffer, n * 4, 1, audio_fp) != 1)
			stop("audio");
		samples -= n;
	}
}


// Called at the end of each emulated frame
void recorder_frame ( const Uint32 *pixels, const int texture_width, const int visible_height, const int frame_usec )
{
	if (video_fp)
		record_video(pixels, texture_width, visible_height);
	if (audio_fp)
		record_audio(frame_usec);
}
