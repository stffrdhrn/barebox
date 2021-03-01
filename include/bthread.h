/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (C) 2021 Ahmad Fatoum, Pengutronix
 */

#ifndef __BTHREAD_H_
#define __BTHREAD_H_

#include <linux/stddef.h>

struct bthread;

struct bthread *bthread_create(int (*threadfn)(void *), void *data, const char *name);
void bthread_free(struct bthread *bthread);

void bthread_schedule(struct bthread *);
void bthread_wake(struct bthread *bthread);
void bthread_suspend(struct bthread *bthread);
int bthread_should_stop(void);
int bthread_stop(struct bthread *bthread);
void bthread_info(void);

#ifdef CONFIG_BTHREAD
void bthread_reschedule(void);
#else
static inline void bthread_reschedule(void)
{
}
#endif

#endif
