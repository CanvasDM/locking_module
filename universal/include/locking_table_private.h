/**
 * @file locking_table_private.h
 *
 * @brief
 *
 * Copyright (c) 2022 Laird Connectivity
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef __LOCKING_TABLE_PRIVATE_H__
#define __LOCKING_TABLE_PRIVATE_H__

/******************************************************************************/
/* Includes                                                                   */
/******************************************************************************/
#include <zephyr.h>
#include <zephyr/types.h>
#include <stddef.h>

#include "locking_defs.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
/* Global Function Prototypes                                                 */
/******************************************************************************/
/**
 * @brief Setup mutexes and semaphores (must be called prior to use).
 */
void locking_table_initialise(void);

/**
 * @brief Reset all semaphore counts to 0 (debug use only).
 */
void locking_table_reset(void);

/**
 * @brief Map ID to table entry
 *
 * @param id ID of lock element.
 * @return const struct locking_table_entry* const
 */
const struct locking_table_entry *const locking_map(locking_id_t id);

/**
 * @brief Calculate index of entry
 *
 * @param entry
 * @return locking_index_t
 */
locking_index_t locking_table_index(
			const struct locking_table_entry *const entry);

#ifdef __cplusplus
}
#endif

#endif /* __LOCKING_TABLE_PRIVATE_H__ */
