#include <ch.h>
#include <hal.h>
#include "periodSense.hpp"
#include <climits>

PeriodSense::PeriodSense(ICUDriver * const _icup, const uint32_t _index):
  icup(_icup)
{
  config = ICUConfig {
    .mode = ICU_INPUT_ACTIVE_HIGH,
    .frequency = 1_mhz,              /* real divider will be calculated dynamically  */
    .width_cb = nullptr,
    .period_cb = [] (ICUDriver *licup) {winAvg[licup->index].push(icuGetWidthX(licup));} ,
    .overflow_cb =  [] (ICUDriver *licup) {licup->hasOverflow = true;
					   palSetLine(LINE_C00_LED1); 	
		    },
    .channel = ICU_CHANNEL_1,
    .dier = 0
  };

  icup->hasOverflow = false;
  icup->index = _index;

  icuStart(icup, &config);
  
  osalDbgAssert((icup->clock ==  TIMER_FREQ_IN) || (icup->clock == (2UL * TIMER_FREQ_IN)),
		 "TIMER_FREQ_IN not compatible with timer source clock");
  osalDbgAssert((_index < TIMER_NUM_INPUT),
		 "not enough index in array, modify NUM_INPUT in hardwareConf.hpp");

  setDivider(TIM_DIVIDER);
  icuStartCapture(icup);
  icuEnableNotifications(icup);
}

uint16_t	PeriodSense::getWidthAverage(void)
{
  if (icup->hasOverflow) {
    return USHRT_MAX;
    icup->hasOverflow = false;
  } else {
    return winAvg[icup->index].getMean();
  }
};


void	PeriodSense::setDivider(const uint16_t divider)
{
  auto cr1 = icup->tim->CR1;
  icup->tim->CR1    = 0;
  icup->tim->CNT    = 0;
  icup->tim->PSC    = divider * (icup->clock / TIMER_FREQ_IN);
  icup->tim->CR1    = cr1;
};

uint32_t	PeriodSense::getERPM(void)
{
  constexpr auto timFreqHz = TIMER_FREQ_IN / TIM_DIVIDER;
  
  const auto period = getWidthAverage();
  const auto rps = timFreqHz / period;
  const auto rpm = rps / 60UL;

  return rpm;
}



CountWinAvg	PeriodSense::winAvg[TIMER_NUM_INPUT];
