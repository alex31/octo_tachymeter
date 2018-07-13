#include <ch.h>
#include <hal.h>
#include "periodSense.hpp"
#include <climits>

PeriodSense::PeriodSense(ICUDriver * const _icup, const icuchannel_t channel):
  icup(_icup)
{
  osalDbgAssert((indexer < TIMER_NUM_INPUT),
		"not enough index in array, modify NUM_INPUT in hardwareConf.hpp");

  icup->index = indexer++;
  if (icup->index > dynSize)
    return;
  
  config = ICUConfig {
    .mode = ICU_INPUT_ACTIVE_HIGH,
    .frequency = 560_khz,              /* real divider will be calculated dynamically  */
    .width_cb = nullptr,
    .period_cb = [] (ICUDriver *licup) {
      winAvg[licup->index].push(icuGetPeriodX(licup));
      licup->hasOverflow = false;
      palClearLine(LINE_LED1); 
    } ,
    .overflow_cb =  [] (ICUDriver *licup) {
      licup->hasOverflow = true;
      palSetLine(LINE_LED1); 	
    },
    .channel = channel,
    .dier = 0
  };

  icup->hasOverflow = false;

  icuStart(icup, &config);
  
  osalDbgAssert((icup->clock ==  TIMER_FREQ_IN) || (icup->clock == (2UL * TIMER_FREQ_IN)),
		 "TIMER_FREQ_IN not compatible with timer source clock");

  setDivider(TIM_DIVIDER);
  icuStartCapture(icup);
  icuEnableNotifications(icup);
}

icucnt_t	PeriodSense::getPeriodAverage(void) const
{
  return (icup->hasOverflow) ? UINT_MAX : winAvg[icup->index].getMean();
};


// constexpr PeriodSense::calculateWidthOneRpm(void)
// {
//   uint64_t num = static_cast<uint64_t> (TIMER_FREQ_IN) / icup->tim->PSC)) * 60UL
// }

void	PeriodSense::setDivider(const uint16_t divider)
{
  const auto cr1 = icup->tim->CR1;
  icup->tim->CR1    = 0;
  icup->tim->CNT    = 0;
  icup->tim->PSC    = divider * (icup->clock / TIMER_FREQ_IN);
  icup->tim->ARR  = 0xFFFF;
  icup->tim->CR1    = cr1;
};

uint32_t	PeriodSense::getRPM(void) const
{
  return  WIDTH_ONE_RPM / getPeriodAverage();
}



CountWinAvg	PeriodSense::winAvg[TIMER_NUM_INPUT];
size_t		PeriodSense::indexer = 0UL;
size_t		PeriodSense::dynSize = 1UL;
