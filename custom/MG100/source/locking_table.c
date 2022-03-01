/**
 * @file locking_table.c
 * @brief
 *
 * Copyright (c) 2022 Laird Connectivity
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/******************************************************************************/
/* Includes                                                                   */
/******************************************************************************/
#include <zephyr.h>
#include <string.h>

#include "locking_table.h"

/* clang-format off */

/******************************************************************************/
/* Local Constant, Macro and Type Definitions                                 */
/******************************************************************************/
/* pystart - locks */
static struct k_mutex adc;
/* pyend */

/******************************************************************************/
/* Global Data Definitions                                                    */
/******************************************************************************/

/**
 * @brief Table shorthand
 *
 * @ref CreateStruct (Python script)
 *
 *.........name...value...
 */
#ifdef CONFIG_ATTR_STRING_NAME
#define LOCK(n) STRINGIFY(n), &n
#else
#define LOCK(n) "", &n
#endif

#define y true
#define n false

/* index....id.name.....................type...count.limit. */
const struct locking_table_entry LOCKING_TABLE[LOCKING_TABLE_SIZE] = {
	/* pystart - locking table */
	[0  ] = { 0  , LOCK(adc)           , LOCKING_TYPE_MUTEX      , .count = 0  , .limit = 0   }
	/* pyend */
};

/**
 * @brief map id to table entry (invalid entries are NULL)
 */
static const struct locking_table_entry * const LOCKING_MAP[] = {
	/* pystart - locking map */
	[0  ] = &LOCKING_TABLE[0  ]
	/* pyend */
};
BUILD_ASSERT(ARRAY_SIZE(LOCKING_MAP) == (LOCKING_TABLE_MAX_ID + 1),
	     "Invalid locking map");

/******************************************************************************/
/* Global Function Definitions                                                */
/******************************************************************************/
void locking_table_initialise(void)
{
	/* pystart - init */
	k_mutex_init(&adc);
	/* pyend */
}

void locking_table_reset(void)
{
	/* pystart - reset */
	/* pyend */
}

const struct locking_table_entry *const locking_map(locking_id_t id)
{
	if (id > LOCKING_TABLE_MAX_ID) {
		return NULL;
	} else {
		return LOCKING_MAP[id];
	}
}

locking_index_t locking_table_index(const struct locking_table_entry *const entry)
{
	__ASSERT(PART_OF_ARRAY(LOCKING_TABLE, entry), "Invalid entry");
	return (entry - &LOCKING_TABLE[0]);
}
