#ifndef _SIMULATOR_USER_ASSERT_H
#define _SIMULATOR_USER_ASSERT_H

#include <stdint.h>

void ShowAssert(const char *file, uint32_t line);

#define ASSERT(expr) ((expr) ? (void)0 : ShowAssert(__FILE__, __LINE__))

#ifdef assert
#undef assert
#endif

#define assert(expr) ASSERT(expr)
#define SHOW_ASSERT() ShowAssert(__FILE__, __LINE__)

#endif
