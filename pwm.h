#pragma once

#include <ch.h>
#include <hal.h>

#ifdef __cplusplus
extern "C" {
#endif

  void launchPwm (void);
  uint32_t pwmGetFreq(void);
  
#ifdef __cplusplus
}
#endif
