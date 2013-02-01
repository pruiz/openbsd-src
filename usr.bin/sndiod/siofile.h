/*	$OpenBSD: siofile.h,v 1.2 2013/02/01 09:06:27 ratchov Exp $	*/
/*
 * Copyright (c) 2008-2012 Alexandre Ratchov <alex@caoua.org>
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
#ifndef SIOFILE_H
#define SIOFILE_H

#include "file.h"

struct dev;

struct siofile_ {
	struct sio_hdl *hdl;
	unsigned int todo;
#ifdef DEBUG
	long long wtime, utime;
	long long sum_wtime, sum_utime;
	int pused, rused, events;
#endif
	struct file *file;
#define DEV_SIO_READ	0
#define DEV_SIO_CYCLE	1
#define DEV_SIO_WRITE	2
	int cstate;
	struct timo watchdog;
};

int dev_sio_open(struct dev *);
void dev_sio_close(struct dev *);
void dev_sio_log(struct dev *);
void dev_sio_start(struct dev *);
void dev_sio_stop(struct dev *);

#endif /* !defined(SIOFILE_H) */
