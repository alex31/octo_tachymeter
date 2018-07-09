#pragma once
#include <ch.h>
#include <hal.h>
#include <math.h>

static constexpr uint32_t operator"" _pwmChannel (unsigned long long int channel)
{
  return channel - 1U;
}
static constexpr uint32_t operator"" _hz (unsigned long long int freq)
{
  return freq;
}
static constexpr uint32_t operator"" _khz (unsigned long long int freq)
{
  return freq * 1000UL;
}
static constexpr uint32_t operator"" _mhz (unsigned long long int freq)
{
  return freq * 1000_khz;
}
static constexpr uint32_t operator"" _percent (unsigned long long int freq)
{
  return freq * 100UL;
}


// USER EDITABLE CONSTANT
static constexpr uint32_t MIN_RPM = 100UL;
static constexpr uint32_t MAX_RPM = 30000UL;
static constexpr uint32_t ERPM_RPM_RATIO     = 6UL;
static constexpr uint32_t TIMER_WIDTH_BITS   = 16UL;
static constexpr uint32_t TIMER_FREQ_IN = STM32_HCLK / 2UL;
static constexpr size_t	  TIMER_NUM_INPUT = 7UL;

// CALCULATED CONSTANTS
static constexpr uint32_t FREQ_AT_MAX_RPM = (MAX_RPM * ERPM_RPM_RATIO) / 60UL;
static constexpr uint32_t FREQ_AT_MIN_RPM = (MIN_RPM * ERPM_RPM_RATIO) / 60UL;
static constexpr uint32_t TICK_AT_MIN_RPM = TIMER_FREQ_IN / FREQ_AT_MIN_RPM;
static constexpr uint32_t TIM_DIVIDER = ceilf (TICK_AT_MIN_RPM / powf(2, TIMER_WIDTH_BITS));
static constexpr uint32_t NB_TICKS_AT_MAX_RPM =  powf(2.0f, TIMER_WIDTH_BITS) * MIN_RPM / MAX_RPM;
static constexpr uint32_t WIDTH_ONE_RPM = TIMER_FREQ_IN  * 60ULL / TIM_DIVIDER / ERPM_RPM_RATIO;
