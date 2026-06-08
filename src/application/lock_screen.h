
#ifndef _LOCK_SCREEN_H
#define _LOCK_SCREEN_H

#include "stdint.h"
#include "stdbool.h"

void LockScreenInit(void);
void ClearLockScreenTime(void);
void LockScreen(void);
void UnlockScreen(void);
bool IsScreenLocked(void);

#endif
