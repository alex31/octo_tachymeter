#include <ch.h>
#include <hal.h>

#pragma once
#if  USE_TIM2_IN_PWM_MODE_FOR_SELF_TESTS

#ifdef __cplusplus
extern "C" {
#endif

  void launchPwm (void);
  uint32_t pwmGetFreq(void);
  
#ifdef __cplusplus
}
#endif

#endif
