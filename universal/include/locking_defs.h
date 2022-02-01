/**
 * @file locking_defs.h
 * @brief Locking definitions
 *
 * Copyright (c) 2022 Laird Connectivity
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef __LOCKING_DEFS_H__
#define __LOCKING_DEFS_H__

/******************************************************************************/
/* Includes                                                                   */
/******************************************************************************/
#include <zephyr.h>
#include <zephyr/types.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
/* Global Constants, Macros and Type Definitions                              */
/******************************************************************************/
typedef uint16_t locking_index_t;
typedef uint16_t locking_id_t;

#define LOCKING_INVALID_ID (UINT16_MAX - 1)

enum locking_type {
	LOCKING_TYPE_UNKNOWN = 0,
	LOCKING_TYPE_ANY,
	LOCKING_TYPE_MUTEX,
	LOCKING_TYPE_SEMAPHORE
};

enum locking_size {
	LOCKING_SIZE_UNKNOWN = 0,
	LOCKING_SIZE_MUTEX = sizeof(struct k_mutex),
	LOCKING_SIZE_SEMAPHORE = sizeof(struct k_sem),
};

typedef struct locking_table_entry lte_t;

struct locking_table_entry {
	const locking_id_t id;
	const char *const name;
	void *const pData;
	const enum locking_type type;
	const uint8_t count;
	const uint8_t limit;
	uint8_t current;
};

#ifdef __cplusplus
}
#endif

#endif /* __LOCKING_DEFS_H__ */
