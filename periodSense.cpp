#include <ch.h>
#include <hal.h>
#include "periodSense.hpp"
#include <climits>

PeriodSense::PeriodSense(ICUDriver * const _icup, const icuchannel_t channel):
  icup(_icup)
{
  osalDbgAssert((indexer < TIMER_NUM_INPUT),
		"not enough index in array, modify NUM_INPUT in hardwareConf.hpp");

  config = ICUConfig {
    .mode = ICU_INPUT_ACTIVE_HIGH,
    .frequency = 1_mhz,              /* real divider will be calculated dynamically  */
    .width_cb = nullptr,
    .period_cb = [] (ICUDriver *licup) {winAvg[licup->index].push(icuGetPeriodX(licup));} ,
    .overflow_cb =  [] (ICUDriver *licup) {licup->hasOverflow = true;
					   palSetLine(LINE_C00_LED1); 	
		    },
    .channel = channel,
    .dier = 0
  };

  icup->hasOverflow = false;
  icup->index = indexer++;

  icuStart(icup, &config);
  
  osalDbgAssert((icup->clock ==  TIMER_FREQ_IN) || (icup->clock == (2UL * TIMER_FREQ_IN)),
		 "TIMER_FREQ_IN not compatible with timer source clock");

  setDivider(TIM_DIVIDER);
  icuStartCapture(icup);
  icuEnableNotifications(icup);
}

uint16_t	PeriodSense::getPeriodAverage(void) const
{
  if (icup->hasOverflow) {
    icup->hasOverflow = false;
    palClearLine(LINE_C00_LED1); 
    return USHRT_MAX;
  } else {
    return winAvg[icup->index].getMean();
  }
};


// constexpr PeriodSense::calculateWidthOneRpm(void)
// {
//   uint64_t num = static_cast<uint64_t> (TIMER_FREQ_IN) / icup->tim->PSC)) * 60UL
// }

void	PeriodSense::setDivider(const uint16_t divider)
{
  auto cr1 = icup->tim->CR1;
  icup->tim->CR1    = 0;
  icup->tim->CNT    = 0;
  icup->tim->PSC    = divider * (icup->clock / TIMER_FREQ_IN);
  icup->tim->CR1    = cr1;
};

uint32_t	PeriodSense::getRPM(void) const
{
  return  WIDTH_ONE_RPM / getPeriodAverage();
}


size_t		PeriodSense::indexer = 0;
CountWinAvg	PeriodSense::winAvg[TIMER_NUM_INPUT]{{ { []{chSysLock();} },
						 { []{chSysUnlock();} }
    }};
