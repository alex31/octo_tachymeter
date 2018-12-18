#pragma once
#include <stdint.h>

#ifndef __gnu_linux__
#include "ch.h"
#include "hal.h"
#endif
/*
#                 _                          _                                           
#                | |                        | |                                          
#                | |__     __ _   _ __    __| |  __      __   __ _   _ __    ___         
#                | '_ \   / _` | | '__|  / _` |  \ \ /\ / /  / _` | | '__|  / _ \        
#                | | | | | (_| | | |    | (_| |   \ V  V /  | (_| | | |    |  __/        
#                |_| |_|  \__,_| |_|     \__,_|    \_/\_/    \__,_| |_|     \___|        
#                                               _                     _            _            
#                                              | |                   (_)          | |           
#                  ___    ___    _ __    ___   | |_    _ __    __ _   _    _ __   | |_          
#                 / __|  / _ \  | '_ \  / __|  | __|  | '__|  / _` | | |  | '_ \  | __|         
#                | (__  | (_) | | | | | \__ \  \ |_   | |    | (_| | | |  | | | | \ |_          
#                 \___|  \___/  |_| |_| |___/   \__|  |_|     \__,_| |_|  |_| |_|  \__|         
*/
enum class SensorType : uint8_t {Hall_effect, Esc_coupler, No_Init};
enum class EepromCommand : uint8_t {Store, Load, Wipe, Erase};
enum class RunningState : uint8_t {Stop, Run, Error};

#ifndef __gnu_linux__


static constexpr uint32_t TIMER_FREQ_IN = STM32_HCLK / 2UL;
static constexpr uint32_t TIMER_DIVIDER_OPTOCOUPLER = 4UL;
static constexpr uint32_t TIMER_FREQ_OPTO = TIMER_FREQ_IN / TIMER_DIVIDER_OPTOCOUPLER;

#if OPTOCOUPLER_ON_BOARD == 1
static constexpr time_msecs_t timerOptoClk2microSeconds (const time_conv_t interval)
{
  return  (((time_conv_t)(interval) * (time_conv_t)1000000) +      
			   (time_conv_t)TIMER_FREQ_OPTO - (time_conv_t)1) /    
			  (time_conv_t)TIMER_FREQ_OPTO;
}

static constexpr time_msecs_t timerOptoClk2milliSeconds (const uint32_t interval)
{
  return   timerOptoClk2microSeconds(interval*1000);
}

static constexpr sysinterval_t microSeconds2timerOptoClk (const time_conv_t usecs)
{
  return  ((sysinterval_t)(((usecs *
			     (time_conv_t)TIMER_FREQ_OPTO) +		
			    (time_conv_t)999999) / (time_conv_t)1000000));
}

static constexpr sysinterval_t milliSeconds2timerOptoClk (const time_conv_t msecs)
{
  return  microSeconds2timerOptoClk(1000 * msecs);
}
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

#if OPTOCOUPLER_ON_BOARD == 1
static constexpr uint32_t operator"" _tim_opto_usec (unsigned long long int duration)
{
  return microSeconds2timerOptoClk(duration);
}

static constexpr uint32_t operator"" _tim_opto_msec (unsigned long long int duration)
{
  return milliSeconds2timerOptoClk(duration);
}
#endif


/*
#                                                    
#                                                    
#                 _   _   ___     ___   _ __         
#                | | | | / __|   / _ \ | '__|        
#                | |_| | \__ \  |  __/ | |           
#                 \__,_| |___/   \___| |_|           
#                            _    _    _              _       _                 
#                           | |  (_)  | |            | |     | |                
#                  ___    __| |   _   | |_     __ _  | |__   | |    ___         
#                 / _ \  / _` |  | |  | __|   / _` | | '_ \  | |   / _ \        
#                |  __/ | (_| |  | |  \ |_   | (_| | | |_) | | |  |  __/        
#                 \___|  \__,_|  |_|   \__|   \__,_| |_.__/  |_|   \___|        
*/
static constexpr uint32_t TIMER_WIDTH_BITS   = 16UL;

// hall effect sensor mode parameters
static constexpr uint32_t MIN_PERIOD_WIDTH_RATIO_TIME10 = 15UL;
static constexpr uint32_t MAX_PERIOD_WIDTH_RATIO_TIME10 = 22UL;

// esc opto coupler parameters
#if OPTOCOUPLER_ON_BOARD == 1
static constexpr uint32_t INACTIVE_DURATION_TO_DETECT_END = 80_tim_opto_usec;
static constexpr uint32_t INACTIVE_DURATION_TO_DETECT_ERROR = 2_tim_opto_msec;
#endif
// parameters that will be used *only* at the first run, 
// before parameters are stored in flash memory.
static constexpr SensorType INIT_SENSOR_TYPE = SensorType::Hall_effect;
static constexpr RunningState INIT_RUNNING_STATE = RunningState::Run;
static constexpr uint32_t INIT_MESS_PER_SECOND = 10;
static constexpr uint32_t INIT_MIN_RPM = 300UL;
static constexpr uint32_t INIT_MAX_RPM = 30000UL;
static constexpr uint32_t INIT_MOTOR_NB_MAGNETS   = 14UL;
static constexpr uint32_t INIT_MOTOR_NB_MOTORS   = 4UL;
static constexpr uint8_t  INIT_WINDOW_FILTER_SIZE = 8;
static constexpr uint8_t  INIT_MEDIAN_FILTER_SIZE = 1;
static constexpr uint8_t  INIT_ERRORS_WINDOW_SIZE = 64;

#if OPTOCOUPLER_ON_BOARD == 1
static_assert(INACTIVE_DURATION_TO_DETECT_ERROR < (1 << TIMER_WIDTH_BITS),
	      "opto coupler INACTIVE_DURATION_TO_DETECT_ERROR to large for timer width");
#endif

#endif // #ifndef __gnu_linux__
