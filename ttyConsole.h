#pragma once

#ifdef __cplusplus
extern "C" {
#endif


// USB : 1, SERIAL : 0
#define CONSOLE_DEV_USB 0

#if CONSOLE_DEV_USB == 0
#define CONSOLE_DEV_SD SD1
#endif


void consoleInit (void);
void consoleLaunch (void);

#ifdef __cplusplus
}
#endif


