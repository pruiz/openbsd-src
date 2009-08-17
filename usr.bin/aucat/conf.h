/*	$OpenBSD: conf.h,v 1.9 2009/08/17 16:17:46 ratchov Exp $	*/
/*
 * Copyright (c) 2008 Alexandre Ratchov <alex@caoua.org>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */
#ifndef CONF_H
#define CONF_H

/*
 * Debug trace levels:
 *
 * 0 - traces are off
 * 1 - init, free, stuff that's done only once
 * 2 - rare real-time events: eof / hup, etc...
 * 3 - poll(), block / unblock state changes
 * 4 - read()/write()
 */
#ifdef DEBUG

/* defined in main.c */
extern int debug_level;

#define DPRINTF(...) DPRINTFN(1, __VA_ARGS__)
#define DPRINTFN(n, ...)					\
	do {							\
		if (debug_level >= (n))				\
			fprintf(stderr, __VA_ARGS__);		\
	} while(0)
#else
#define DPRINTF(...) do {} while(0)
#define DPRINTFN(n, ...) do {} while(0)
#endif

/*
 * Number of blocks in the device play/record buffers.  Because Sun API
 * cannot notify apps of the current positions, we have to use all N
 * buffers devices blocks plus one extra block, to make write() block,
 * so that poll() can return the exact postition.
 */
#define DEV_NBLK 2

/*
 * Number of blocks in the wav-file i/o buffers.
 */
#define WAV_NBLK 6

/*
 * socket and option names
 */
#define DEFAULT_MIDITHRU	"midithru"
#define DEFAULT_SOFTAUDIO	"softaudio"
#define DEFAULT_OPT		"default"

/*
 * MIDI buffer size
 */
#define MIDI_BUFSZ		3125	/* 1 second */

#endif /* !defined(CONF_H) */
