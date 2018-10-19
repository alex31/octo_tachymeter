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

enum class SensorType {Hall_effect, Esc_coupler, No_Init};
enum class RunningState {Stop, Run, Error};

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
static constexpr SensorType INIT_SENSOR_TYPE = SensorType::Hall_effect;
static constexpr RunningState INIT_RUNNING_STATE = RunningState::Run;

static constexpr uint32_t INIT_MIN_RPM = 100UL;
static constexpr uint32_t INIT_MAX_RPM = 30000UL;
static constexpr uint32_t INIT_MOTOR_NB_MAGNETS   = 14UL;
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
 


