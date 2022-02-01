/**
 * @file locking.h
 * @brief
 *
 * Copyright (c) 2022 Laird Connectivity
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef __LOCKING_H__
#define __LOCKING_H__

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
/* Includes                                                                   */
/******************************************************************************/
#include <stdint.h>
#include <stddef.h>
#include <sys_clock.h>

#ifdef CONFIG_LOCKING_SHELL
#include <shell/shell.h>
#endif

#include "locking_table.h"

/******************************************************************************/
/* Function Definitions                                                       */
/******************************************************************************/
/**
 * @brief Get the type of the lock.
 *
 * @param id A lock ID.
 *
 * @retval type Type of variable.
 */
enum locking_type locking_get_type(locking_id_t id);

/**
 * @brief Helper function.
 *
 * @param id A lock ID.
 *
 * @retval true if id is valid, false otherwise.
 */
bool locking_valid_id(locking_id_t id);

/**
 * @brief Get name of lock (returns empty string if LOCKING_STRING_NAME is not
 *        enabled)
 *
 * @param id A lock ID.
 *
 * @retval char Name of lock if valid, empty string otherwise.
 */
const char *locking_get_name(locking_id_t id);

/**
 * @brief Take a mutex or semaphore lock, waiting up to specified time for it
 *        to become available.
 *
 * @param id A lock ID.
 * @param wait_time The time to wait to take the lock.
 *
 * @retval negative error code, 0 on success.
 */
int locking_take(locking_id_t id, k_timeout_t wait_time);

/**
 * @brief Give a mutex or semaphore lock.
 *
 * @param id A lock ID.
 *
 * @retval negative error code, size of value on return.
 */
int locking_give(locking_id_t id);

#ifdef CONFIG_LOCKING_SHELL
/**
 * @brief Get the id of a lock
 *
 * @param name Name of the lock.
 *
 * @retval locking_id_t ID of lock.
 */
locking_id_t locking_get_id(const char *name);

/**
 * @brief Print the details of a lock
 *
 * @param shell Pointer to shell instance.
 * @param id A lock ID.
 *
 * @retval negative error code, 0 on success.
 */
int locking_show(const struct shell *shell, locking_id_t id);

/**
 * @brief Print all locks to the console using system workq.
 *
 * @param shell Pointer to shell instance.
 *
 * @retval negative error code, 0 on success.
 *
 * @note For remote execution using mcumgr, SHELL_BACKEND_DUMMY_BUF_SIZE
 * must be set large enough to display all values.
 */
int locking_show_all(const struct shell *shell);
#endif /* CONFIG_LOCKING_SHELL */

#ifdef __cplusplus
}
#endif

#endif /* __LOCKING_H__ */
