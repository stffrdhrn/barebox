/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (C) 2021 Ahmad Fatoum, Pengutronix
 *
 * ASAN bookkeeping based on Qemu coroutine-ucontext.c
 */

/* To avoid future issues; fortify doesn't like longjmp up the call stack */
#ifndef __NO_FORTIFY
#define __NO_FORTIFY
#endif

#include <common.h>
#include <bthread.h>
#include <asm/setjmp.h>
#include <linux/overflow.h>

struct bthread {
	int (*threadfn)(void *);
	union {
		void *data;
		int ret;
	};
	char *name;
	jmp_buf jmp_buf;
	void *stack;
	u32 stack_size;
	struct list_head list;
#ifdef CONFIG_ASAN
	void *fake_stack_save;
#endif
	u16 awake :1;
	u16 should_stop :1;
	u16 has_stopped :1;
	u8 stack_space[] __aligned(16);
} main_thread = {
	.list = LIST_HEAD_INIT(main_thread.list),
	.name = "main",
};

#define next(bthread)	list_next_entry(bthread, list)
#define prev(bthread)	list_prev_entry(bthread, list)
#define empty(bthread)	list_empty(&(bthread)->list)

static struct bthread *current = &main_thread;

/*
 * When using ASAN, it needs to be told when we switch stacks.
 */
static void start_switch_fiber(struct bthread *, bool terminate_old);
static void finish_switch_fiber(struct bthread *);

static void __noreturn bthread_trampoline(void)
{
	finish_switch_fiber(current);
	bthread_schedule(&main_thread);

	current->ret = current->threadfn(current->data);

	current->has_stopped = true;
	bthread_suspend(current);

	current = &main_thread;
	start_switch_fiber(&main_thread, true);
	longjmp(main_thread.jmp_buf, 1);
}

void bthread_free(struct bthread *bthread)
{
	free(bthread->name);
	free(bthread);
}

struct bthread *bthread_create(int (*threadfn)(void *), void *data, const char *name)
{
	struct bthread *bthread;
	int ret;

	bthread = malloc(struct_size(bthread, stack_space, CONFIG_STACK_SIZE));
	if (!bthread)
		return NULL;

	memset(bthread, 0, sizeof(*bthread));

	bthread->stack = bthread->stack_space;
	bthread->stack_size = CONFIG_STACK_SIZE;
	bthread->threadfn = threadfn;
	bthread->data = data;
	bthread->name = strdup(name);

	/* set up bthread context with the new stack */
	ret = initjmp(bthread->jmp_buf, bthread_trampoline,
		      bthread->stack + CONFIG_STACK_SIZE);
	if (ret) {
		bthread_free(bthread);
		return NULL;
	}

	return bthread;
}

void bthread_wake(struct bthread *bthread)
{
	if (bthread->awake)
		return;
	list_add(&bthread->list, &main_thread.list);
	bthread->awake = true;
}

void bthread_suspend(struct bthread *bthread)
{
	if (!bthread->awake || bthread == &main_thread)
		return;
	bthread->awake = false;
	list_del(&bthread->list);
}

int bthread_stop(struct bthread *bthread)
{
	bthread->should_stop = true;

	while (!bthread->has_stopped)
		bthread_reschedule();

	return bthread->ret;
}

int bthread_should_stop(void)
{
	if (current == &main_thread)
		return -EINTR;
	bthread_schedule(&main_thread);
	return current->should_stop;
}

void bthread_info(void)
{
	struct bthread *bthread;

	printf("Registered secondary barebox threads:\n");

	if (empty(&main_thread)) {
		printf("<none>\n");
		return;
	}

	list_for_each_entry(bthread, &main_thread.list, list)
		printf("%s\n", bthread->name);
}

void bthread_reschedule(void)
{
	struct bthread *to, *tmp;

	if (current != &main_thread) {
		bthread_schedule(&main_thread);
		return;
	}

	list_for_each_entry_safe(to, tmp, &main_thread.list, list)
		bthread_schedule(to);
}

void bthread_schedule(struct bthread *to)
{
	struct bthread *from = current;
	int ret;

	start_switch_fiber(to, false);

	ret = setjmp(from->jmp_buf);
	if (ret == 0) {
		current = to;
		longjmp(to->jmp_buf, 1);
	}

	finish_switch_fiber(from);
}

#ifdef CONFIG_ASAN

void __sanitizer_start_switch_fiber(void **fake_stack_save, const void *bottom, size_t size);
void __sanitizer_finish_switch_fiber(void *fake_stack_save, const void **bottom_old, size_t *size_old);

static void finish_switch_fiber(struct bthread *bthread)
{
	const void *bottom_old;
	size_t size_old;

	__sanitizer_finish_switch_fiber(bthread->fake_stack_save, &bottom_old, &size_old);

	if (!main_thread.stack) {
		main_thread.stack = (void *)bottom_old;
		main_thread.stack_size = size_old;
	}
}

static void start_switch_fiber(struct bthread *to, bool terminate_old)
{
	__sanitizer_start_switch_fiber(terminate_old ? &to->fake_stack_save : NULL,
				       to->stack, to->stack_size);
}

#else

static void finish_switch_fiber(struct bthread *bthread)
{
}

static void start_switch_fiber(struct bthread *to, bool terminate_old)
{
}

#endif
