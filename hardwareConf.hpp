#pragma once
#include <stdint.h>

enum class SensorType : uint8_t {Hall_effect, Esc_coupler, No_Init};
enum class RunningState : uint8_t {Stop, Run, Error};

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
static constexpr uint32_t INIT_MESS_PER_SECOND = 10;

static constexpr uint32_t INIT_MIN_RPM = 300UL;
static constexpr uint32_t INIT_MAX_RPM = 30000UL;
static constexpr uint32_t INIT_MOTOR_NB_MAGNETS   = 14UL;
static constexpr uint32_t INIT_MOTOR_NB_MOTORS   = 4UL;
static constexpr uint32_t TIMER_WIDTH_BITS   = 16UL;
static constexpr uint32_t MIN_PERIOD_WIDTH_RATIO_TIME10 = 15UL;
static constexpr uint32_t MAX_PERIOD_WIDTH_RATIO_TIME10 = 22UL;


 


