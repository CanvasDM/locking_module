/**
 * @file locking_shell.c
 * @brief
 *
 * Copyright (c) 2022 Laird Connectivity
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/******************************************************************************/
/* Includes                                                                   */
/******************************************************************************/
#include <shell/shell.h>
#include <init.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include "locking.h"
#include "locking_table_private.h"

/******************************************************************************/
/* Global Constants, Macros and Type Definitions                              */
/******************************************************************************/
#define DEFAULT_WAIT_TIME_SECONDS 3

/******************************************************************************/
/* Local Function Prototypes                                                  */
/******************************************************************************/
static int ats_show_cmd(const struct shell *shell, size_t argc, char **argv);
static int ats_get_cmd(const struct shell *shell, size_t argc, char **argv);

#ifdef CONFIG_LOCKING_SHELL_MANIPULATION
static int ats_take_cmd(const struct shell *shell, size_t argc, char **argv);
static int ats_give_cmd(const struct shell *shell, size_t argc, char **argv);
static int ats_reset_cmd(const struct shell *shell, size_t argc, char **argv);
#endif

static int locking_shell_init(const struct device *device);

/******************************************************************************/
/* Global Function Definitions                                                */
/******************************************************************************/
SHELL_STATIC_SUBCMD_SET_CREATE(
	sub_attr,
	SHELL_CMD(show, NULL, "Display details on all locks", ats_show_cmd),
	SHELL_CMD(get, NULL, "Get details of a lock", ats_get_cmd),
#ifdef CONFIG_LOCKING_SHELL_MANIPULATION
	SHELL_CMD(give, NULL, "Give mutex/semaphore lock", ats_give_cmd),
	SHELL_CMD(take, NULL, "Take mutex/semaphore lock", ats_take_cmd),
	SHELL_CMD(reset, NULL, "Reset all locks", ats_reset_cmd),
#endif
	SHELL_SUBCMD_SET_END);

SHELL_CMD_REGISTER(locking, &sub_attr, "Locking Utilities", NULL);

SYS_INIT(locking_shell_init, APPLICATION, 99);

/******************************************************************************/
/* Local Function Definitions                                                 */
/******************************************************************************/
/* Strings can have numbers, but most likely it won't be the first char. */
static bool is_string(const char *str)
{
	if (isdigit((int)str[0])) {
		return false;
	}
	return true;
}

static locking_id_t get_id(const char *str)
{
	locking_id_t id = 0;

	if (is_string(str)) {
		id = locking_get_id(str);
	} else {
		id = (locking_id_t)strtoul(str, NULL, 0);
	}
	return id;
}

static int ats_show_cmd(const struct shell *shell, size_t argc, char **argv)
{
	ARG_UNUSED(argc);
	ARG_UNUSED(argv);
	locking_show_all(shell);
	return 0;
}

static int ats_get_cmd(const struct shell *shell, size_t argc, char **argv)
{
	int r = 0;
	locking_id_t id = 0;

	if ((argc == 2) && (argv[1] != NULL)) {
		id = get_id(argv[1]);
		r = locking_show(shell, id);
		if (r != 0) {
			shell_error(shell, "Error getting lock details: %d", r);
		}
	} else {
		shell_error(shell, "Unexpected parameters");
		r = -EINVAL;
	}

	return r;
}

#ifdef CONFIG_LOCKING_SHELL_MANIPULATION
static int ats_give_cmd(const struct shell *shell, size_t argc, char **argv)
{
	int r = -EPERM;
	locking_id_t id = 0;

	if ((argc == 2) && (argv[1] != NULL)) {
		id = get_id(argv[1]);
		r = locking_give(id);

		if (r == 0) {
			shell_print(shell, "Lock %d (%s) given", id,
				    locking_get_name(id));
		} else {
			shell_error(shell, "Lock %d (%s) give failed: %d", id,
				    locking_get_name(id), r);
		}
	} else {
		shell_error(shell, "Unexpected parameters");
		return -EINVAL;
	}
	return 0;
}

static int ats_take_cmd(const struct shell *shell, size_t argc, char **argv)
{
	int r = -EPERM;
	locking_id_t id = 0;
	uint32_t wait_time = DEFAULT_WAIT_TIME_SECONDS;

	if ((argc == 2 || argc == 3) && (argv[1] != NULL)) {
		id = get_id(argv[1]);

		if (argc == 3) {
			if (argv[2] == NULL) {
				shell_error(shell, "Unexpected parameters");
				return -EINVAL;
			}
			wait_time = strtoul(argv[2], NULL, 0);
		}

		r = locking_take(id, K_SECONDS(wait_time));

		if (r == 0) {
			shell_print(shell, "Lock %d (%s) taken", id,
				    locking_get_name(id));
		} else {
			shell_error(shell, "Lock %d (%s) take failed: %d", id,
				    locking_get_name(id), r);
		}
	} else {
		shell_error(shell, "Unexpected parameters");
		return -EINVAL;
	}

	return 0;
}

static int ats_reset_cmd(const struct shell *shell, size_t argc, char **argv)
{
	locking_table_reset();
	shell_print(shell, "Lock reset complete");

	return 0;
}
#endif

static int locking_shell_init(const struct device *device)
{
	ARG_UNUSED(device);

	return 0;
}
