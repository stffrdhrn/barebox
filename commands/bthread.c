/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (C) 2021 Ahmad Fatoum, Pengutronix
 */

#include <bthread.h>
#include <errno.h>
#include <malloc.h>
#include <stdio.h>
#include <command.h>
#include <getopt.h>
#include <clock.h>

static int bthread_time(void)
{
	uint64_t start = get_time_ns();
	int i = 0;

	/*
	 * How many background tasks can we have in one second?
	 *
	 * A low number here may point to problems with bthreads taking too
	 * much time.
	 */
	while (!is_timeout(start, SECOND))
		i++;

	return i;
}

static int bthread_infinite(void *data)
{
	while (!bthread_should_stop())
		;

	return 0;
}

static int bthread_isolated_time(void)
{
	uint64_t start = get_time_ns();
	struct bthread *bthread;
	int i = 0;

	bthread = bthread_create(bthread_infinite, NULL, "infinite");
	if (!bthread)
		return -ENOMEM;

	bthread_wake(bthread);

	/*
	 * How many context switches can we do in one second?
	 *
	 * A low number here may point to problems with bthreads taking too
	 * much time.
	 */
	while (!is_timeout_non_interruptible(start, SECOND)) {
		bthread_schedule(bthread);
		i += 2;
	}

	bthread_stop(bthread);
	bthread_free(bthread);

	return i;
}

static int bthread_printer(void *arg)
{
	volatile u64 start;
	volatile int i = 0;
	start = get_time_ns();

	while (!bthread_should_stop()) {
		if (!is_timeout_non_interruptible(start, 225 * MSECOND))
			continue;

		printf("%s yield #%d\n", __func__, ++i);
		start = get_time_ns();
	}

	return i;
}

BAREBOX_CMD_HELP_START(bthread)
	BAREBOX_CMD_HELP_TEXT("print info about registered barebox threads")
	BAREBOX_CMD_HELP_TEXT("")
	BAREBOX_CMD_HELP_TEXT("Options:")
	BAREBOX_CMD_HELP_OPT ("-i", "Print information about registered bthreads")
	BAREBOX_CMD_HELP_OPT ("-t", "measure how many bthreads we currently run in 1s")
	BAREBOX_CMD_HELP_OPT ("-c", "count maximum context switches in 1s")
	BAREBOX_CMD_HELP_OPT ("-v", "verify correct bthread operation")
	BAREBOX_CMD_HELP_END

static int do_bthread(int argc, char *argv[])
{
	struct bthread *bthread = NULL;
	int ret, opt;
	int yields;

	while ((opt = getopt(argc, argv, "itcv")) > 0) {
		switch (opt) {
		case 'i':
			bthread_info();
			return 0;
		case 'c':
			yields = bthread_isolated_time();
			printf("%d bthread context switches possible in 1s\n", yields);
			break;
		case 'v':
			bthread = bthread_create(bthread_printer, NULL, "bthread");
			if (!bthread)
				return -ENOMEM;

			bthread_wake(bthread);

			/* fallthrough */
		case 't':
			yields = bthread_time();
			printf("%d bthread yield calls in 1s\n", yields);
		}

		if (bthread) {
			ret = bthread_stop(bthread);
			bthread_free(bthread);

			if (ret != 4 || yields < ret)
				return COMMAND_ERROR;
		}

		return 0;
	}

	return COMMAND_ERROR_USAGE;
}

BAREBOX_CMD_START(bthread)
	.cmd = do_bthread,
	BAREBOX_CMD_DESC("print info about registered bthreads")
	BAREBOX_CMD_GROUP(CMD_GRP_MISC)
	BAREBOX_CMD_HELP(cmd_bthread_help)
BAREBOX_CMD_END
