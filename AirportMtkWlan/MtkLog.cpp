//
//  MtkLog.cpp
//  AirportMtkWlan
//
//  In-memory driver log, as in SimpleMtkWlan.cpp: every XYLog/printf from
//  the MediaTek HAL, net80211 and the AirportItlwm layer goes through
//  smtw_log() (IOLog plus a copy), and the copy is published as the
//  "SimpleMtkWlanLog" property of the PCI device so it can be read with
//  ioreg (tools/mtklog.sh) when IOLog output is not visible.
//

#include <IOKit/IOLib.h>
#include <IOKit/IOService.h>
#include <libkern/c++/OSString.h>
#include <stdarg.h>

static char gSmtwLog[65536];
static size_t gSmtwLogLen;
static IOSimpleLock *gSmtwLogLock;

void smtw_log_init(void)
{
    if (gSmtwLogLock == NULL)
        gSmtwLogLock = IOSimpleLockAlloc();
}

extern "C" void smtw_log(const char *fmt, ...)
{
    char line[512];
    va_list ap;
    va_start(ap, fmt);
    int n = vsnprintf(line, sizeof(line), fmt, ap);
    va_end(ap);
    if (n < 0)
        return;
    size_t len = strlen(line);
    IOLog("%s", line);
    if (gSmtwLogLock == NULL)
        return;
    IOInterruptState is = IOSimpleLockLockDisableInterrupt(gSmtwLogLock);
    if (gSmtwLogLen + len >= sizeof(gSmtwLog)) {
        /* keep the newest half */
        size_t keep = sizeof(gSmtwLog) / 2;
        if (keep > gSmtwLogLen)
            keep = gSmtwLogLen;
        memmove(gSmtwLog, gSmtwLog + gSmtwLogLen - keep, keep);
        gSmtwLogLen = keep;
    }
    if (len < sizeof(gSmtwLog) - gSmtwLogLen) {
        memcpy(gSmtwLog + gSmtwLogLen, line, len);
        gSmtwLogLen += len;
        gSmtwLog[gSmtwLogLen] = '\0';
    }
    IOSimpleLockUnlockEnableInterrupt(gSmtwLogLock, is);
}

void smtw_log_publish(IOService *target)
{
    if (target == NULL || gSmtwLogLock == NULL)
        return;
    char *copy = (char *)IOMalloc(sizeof(gSmtwLog));
    if (copy == NULL)
        return;
    IOInterruptState is = IOSimpleLockLockDisableInterrupt(gSmtwLogLock);
    memcpy(copy, gSmtwLog, gSmtwLogLen + 1);
    IOSimpleLockUnlockEnableInterrupt(gSmtwLogLock, is);
    OSString *str = OSString::withCString(copy);
    IOFree(copy, sizeof(gSmtwLog));
    if (str) {
        target->setProperty("SimpleMtkWlanLog", str);
        str->release();
    }
}
