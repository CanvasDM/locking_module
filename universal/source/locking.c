/**
 * @file locking.c
 * @brief
 *
 * Copyright (c) 2022 Laird Connectivity
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <logging/log.h>
LOG_MODULE_REGISTER(locking, CONFIG_LOCKING_LOG_LEVEL);

#define LOG_SHOW LOG_INF

/******************************************************************************/
/* Includes                                                                   */
/******************************************************************************/
#include <zephyr.h>
#include <init.h>
#include <stdio.h>
#include <logging/log_ctrl.h>
#include <sys/util.h>
#include <sys/crc.h>

#include "locking_table.h"
#include "locking_table_private.h"
#include "locking.h"

/******************************************************************************/
/* Global Constants, Macros and type Definitions                              */
/******************************************************************************/
#define GIVE_MUTEX(m) k_mutex_unlock(&m)

#define LOCKING_ENTRY_DECL(x)                                                  \
	const struct locking_table_entry *const entry = locking_map(x);

static const char EMPTY_STRING[] = "";

#if defined(CONFIG_THREAD_MAX_NAME_LEN) && CONFIG_THREAD_MAX_NAME_LEN > 10
#define OUTPUT_THREAD_NAME_SIZE CONFIG_THREAD_MAX_NAME_LEN
#else
#define OUTPUT_THREAD_NAME_SIZE 11
#endif

/******************************************************************************/
/* Global Data Definitions                                                    */
/******************************************************************************/
extern const struct locking_table_entry LOCKING_TABLE[LOCKING_TABLE_SIZE];

/******************************************************************************/
/* Local Function Prototypes                                                  */
/******************************************************************************/
#ifdef CONFIG_LOCKING_VERBOSE_DEBUGGING
static int show(const lte_t *const entry);
#endif

#if defined(CONFIG_LOCKING_VERBOSE_DEBUGGING) || defined(CONFIG_LOCKING_SHELL)
static const char *plural(uint8_t input);
static void get_mutex_thread_name(struct k_thread *mutex_owner_thread,
				  uint8_t *buffer, uint8_t buffer_size);
#endif

static int locking_init(const struct device *device);

extern void locking_table_initialise(void);

/******************************************************************************/
/* Global Function Definitions                                                */
/******************************************************************************/
SYS_INIT(locking_init, APPLICATION, CONFIG_LOCKING_INIT_PRIORITY);

enum locking_type locking_get_type(locking_id_t id)
{
	LOCKING_ENTRY_DECL(id);

	if (entry != NULL) {
		return entry->type;
	} else {
		return LOCKING_TYPE_UNKNOWN;
	}
}

bool locking_valid_id(locking_id_t id)
{
	LOCKING_ENTRY_DECL(id);

	return (entry != NULL);
}

const char *locking_get_name(locking_id_t id)
{
	const char *s = EMPTY_STRING;
#ifdef CONFIG_LOCKING_STRING_NAME
	LOCKING_ENTRY_DECL(id);

	if (entry != NULL) {
		s = (const char *)entry->name;
	}
#endif

	return s;
}

#ifdef CONFIG_LOCKING_SHELL

locking_id_t locking_get_id(const char *name)
{
#ifdef CONFIG_LOCKING_STRING_NAME
	locking_index_t i;

	for (i = 0; i < LOCKING_TABLE_SIZE; i++) {
		if (strcmp(name, LOCKING_TABLE[i].name) == 0) {
			return LOCKING_TABLE[i].id;
		}
	}
#endif

	return LOCKING_INVALID_ID;
}

static int shell_show(const struct shell *shell, const lte_t *const entry)
{
	int r;
	uint8_t thread_name_buffer[OUTPUT_THREAD_NAME_SIZE];
	struct k_mutex *tmp_mutex;
	thread_name_buffer[0] = 0;

	switch (entry->type) {
	case LOCKING_TYPE_MUTEX:
		tmp_mutex = (struct k_mutex*)entry->pData;
		get_mutex_thread_name(tmp_mutex->owner,
				      thread_name_buffer,
				      sizeof(thread_name_buffer));

		shell_print(shell, CONFIG_LOCKING_SHOW_FMT
			    ": mutex (%d lock%s held%s%s)",
			    entry->id, entry->name, tmp_mutex->lock_count,
			    plural(tmp_mutex->lock_count),
			    (tmp_mutex->lock_count == 0 ? "" : " by "),
			    thread_name_buffer
			   );
		break;

	case LOCKING_TYPE_SEMAPHORE:
		r = k_sem_count_get(entry->pData);
		shell_print(shell, CONFIG_LOCKING_SHOW_FMT
			    ": semaphore (%d of %d lock%s free)",
			    entry->id, entry->name, r, entry->limit,
			    plural(entry->limit));
		break;

	default:
		shell_print(shell, CONFIG_LOCKING_SHOW_FMT
			    ": unknown type %d", entry->id, entry->name,
			    entry->type);
		break;
	}

	return 0;
}

int locking_show(const struct shell *shell, locking_id_t id)
{
	int r = -EINVAL;
	LOCKING_ENTRY_DECL(id);

	if (entry != NULL) {
		r = shell_show(shell, entry);
	}

	return r;
}

int locking_show_all(const struct shell *shell)
{
	locking_index_t i;

	for (i = 0; i < LOCKING_TABLE_SIZE; i++) {
		(void)shell_show(shell, &LOCKING_TABLE[i]);
	}

	return 0;
}

#endif /* CONFIG_LOCKING_SHELL */

/******************************************************************************/
/* Local Function Definitions                                                 */
/******************************************************************************/
#ifdef CONFIG_LOCKING_VERBOSE_DEBUGGING
static int show(const lte_t *const entry)
{
	int r;
	uint8_t thread_name_buffer[OUTPUT_THREAD_NAME_SIZE];
	struct k_mutex *tmp_mutex;
	thread_name_buffer[0] = 0;

	switch (entry->type) {
	case LOCKING_TYPE_MUTEX:
		tmp_mutex = (struct k_mutex*)entry->pData;
		get_mutex_thread_name(tmp_mutex->owner,
				      thread_name_buffer,
				      sizeof(thread_name_buffer));

		LOG_SHOW(CONFIG_LOCKING_SHOW_FMT ": mutex (%d lock%s held%s%s)",
			 entry->id, entry->name, tmp_mutex->lock_count,
			 plural(tmp_mutex->lock_count),
			 (tmp_mutex->lock_count == 0 ? "" : " by "),
			 thread_name_buffer
			);
		break;

	case LOCKING_TYPE_SEMAPHORE:
		r = k_sem_count_get(entry->pData);
		LOG_SHOW(CONFIG_LOCKING_SHOW_FMT
			 ": semaphore (%d of %d lock%s free)",
			 entry->id, entry->name, r, entry->limit,
			 plural(entry->limit));
		break;

	default:
		LOG_SHOW(CONFIG_LOCKING_SHOW_FMT ": unknown type %d",
			 entry->id, entry->name, entry->type);
		break;
	}

	return 0;
}
#endif

#if defined(CONFIG_LOCKING_VERBOSE_DEBUGGING) || defined(CONFIG_LOCKING_SHELL)
static const char *plural(uint8_t input)
{
	if (input == 1) {
		return EMPTY_STRING;
	}

	return "s";
}

#ifdef CONFIG_THREAD_NAME
static void get_mutex_thread_name(struct k_thread *mutex_owner_thread,
				  uint8_t *buffer, uint8_t buffer_size)
{
	if (mutex_owner_thread == NULL) {
		buffer[0] = 0;
		return;
	} else if (mutex_owner_thread->name == NULL) {
		snprintk(buffer, buffer_size, "%p", mutex_owner_thread);
		return;
	}

	strncpy(buffer, mutex_owner_thread->name, buffer_size);
	buffer[(buffer_size - 1)] = 0;
}
#else
static void get_mutex_thread_name(struct k_thread *mutex_owner_thread,
				  uint8_t *buffer, uint8_t buffer_size)
{
	if (mutex_owner_thread == NULL) {
		buffer[0] = 0;
		return;
	}

	snprintk(buffer, buffer_size, "%p", mutex_owner_thread);
}
#endif

#endif

int locking_take(locking_id_t id, k_timeout_t wait_time)
{
	int r = -EINVAL;
	LOCKING_ENTRY_DECL(id);

	if (entry != NULL) {
		if (entry->type == LOCKING_TYPE_MUTEX) {
			r = k_mutex_lock(entry->pData, wait_time);
		} else if (entry->type == LOCKING_TYPE_SEMAPHORE) {
			r = k_sem_take(entry->pData, wait_time);
		}

#ifdef CONFIG_LOCKING_VERBOSE_DEBUGGING
		show(entry);
#endif
	}

	return r;
}

int locking_give(locking_id_t id)
{
	int r = -EINVAL;
	LOCKING_ENTRY_DECL(id);

	if (entry != NULL) {
		if (entry->type == LOCKING_TYPE_MUTEX) {
			r = k_mutex_unlock(entry->pData);
		} else if (entry->type == LOCKING_TYPE_SEMAPHORE) {
			k_sem_give(entry->pData);
			r = 0;
		}

#ifdef CONFIG_LOCKING_VERBOSE_DEBUGGING
		show(entry);
#endif
	}

	return r;
}


/******************************************************************************/
/* SYS INIT                                                                   */
/******************************************************************************/
static int locking_init(const struct device *device)
{
	ARG_UNUSED(device);

	locking_table_initialise();

	return 0;
}

