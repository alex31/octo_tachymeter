#pragma once
#include <ch.h>
#include <hal.h>
#include <math.h>
#include "gpioBus.hpp"
#include <utility>

#if  USE_TIM2_IN_PWM_MODE_FOR_SELF_TESTS
#define ICU_NUMBER_OF_ENTRIES 7
#else
#define ICU_NUMBER_OF_ENTRIES 8
#endif

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
static constexpr uint32_t MOTOR_NB_MAGNETS   = 14UL;
static constexpr uint32_t TIMER_WIDTH_BITS   = 16UL;
static constexpr uint32_t TIMER_FREQ_IN = STM32_HCLK / 2UL;
static constexpr uint32_t MIN_PERIOD_WIDTH_RATIO_TIME10 = 15UL;
static constexpr uint32_t MAX_PERIOD_WIDTH_RATIO_TIME10 = 22UL;
// 3 pins to code number of motor : BUS_NBM[0..3]
// 1 pin to code HALL or ESC mode
static constexpr std::array<GpioMask, 2> JUMPER_BUSES = {{
    {GPIOB_BASE, (1<<BUS_NBM0) | (1<<BUS_NBM1) | (1<<BUS_NBM2)},
    {GPIOB_BASE, (1<<BUS_HALL_OR_ESC)}
  }};


using IcuEntry = std::pair<ICUDriver * const, const icuchannel_t>;

static constexpr std::array<IcuEntry, ICU_NUMBER_OF_ENTRIES> ICU_TIMER = {{
      {&ICUD1, ICU_CHANNEL_1},  // 168
#if USE_TIM2_IN_PWM_MODE_FOR_SELF_TESTS == 0
      {&ICUD2, ICU_CHANNEL_1},  // 84
#endif
      {&ICUD3, ICU_CHANNEL_1},  // 84
      {&ICUD4, ICU_CHANNEL_1},  // 84
      {&ICUD5, ICU_CHANNEL_1},  // 84
      {&ICUD8, ICU_CHANNEL_1},  // 168
      {&ICUD9, ICU_CHANNEL_1},  // 168
      {&ICUD12, ICU_CHANNEL_1}, // 168
    }} ;
 

// CALCULATED CONSTANTS
static constexpr uint32_t FREQ_AT_MAX_RPM = (MAX_RPM * MOTOR_NB_MAGNETS) / 60UL;
static constexpr uint32_t FREQ_AT_MIN_RPM = (MIN_RPM * MOTOR_NB_MAGNETS) / 60UL;
static constexpr uint32_t TICK_AT_MIN_RPM = TIMER_FREQ_IN / FREQ_AT_MIN_RPM;
static constexpr uint32_t TIM_DIVIDER = ceilf (TICK_AT_MIN_RPM / powf(2, TIMER_WIDTH_BITS));
static constexpr uint32_t NB_TICKS_AT_MAX_RPM =  powf(2.0f, TIMER_WIDTH_BITS) * MIN_RPM / MAX_RPM;
static constexpr uint32_t WIDTH_ONE_RPM = TIMER_FREQ_IN  * 60ULL / TIM_DIVIDER / MOTOR_NB_MAGNETS;
