#include <ch.h>
#include <hal.h>
#include "periodSense.hpp"
#include <climits>

void PeriodSense::setIcu(ICUDriver * const _icup, const icuchannel_t channel)
{
  osalDbgAssert((indexer < ICU_NUMBER_OF_ENTRIES),
		"not enough index in array, modify ICU_NUMBER_OF_ENTRIES in hardwareConf.hpp");

  icup = _icup;
  icup->index = indexer++;
  
  config = ICUConfig {
    .mode = ICU_INPUT_ACTIVE_HIGH,
    .frequency = 1_mhz,              /* real divider will be calculated dynamically  */
    .width_cb = nullptr,
    .period_cb = [] (ICUDriver *licup) {
      const uint32_t p = icuGetPeriodX(licup);
      const uint32_t w = icuGetWidthX(licup);
      const uint32_t r10 = (p * 10) / w;
      if ((r10 <= MAX_PERIOD_WIDTH_RATIO_TIME10) &&
	  (r10 >= MIN_PERIOD_WIDTH_RATIO_TIME10)) {
	winAvg[licup->index].push(p);
	winErr[licup->index].push(0);
      } else {
	winErr[licup->index].push(1);
      }
      licup->hasOverflow = false;
#ifdef TRACE
      palClearLine(LINE_LED1);
#endif
    } ,
    .overflow_cb =  [] (ICUDriver *licup) {
      winErr[licup->index].push(1);
      licup->hasOverflow = true;
#ifdef TRACE
      palSetLine(LINE_LED1); 	
#endif
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
  icup->tim->PSC    = (divider * (icup->clock / TIMER_FREQ_IN)) - 1;
  icup->tim->ARR  = 0xFFFF;
  icup->tim->CR1    = cr1;
};

uint32_t	PeriodSense::getRPM(void) const
{
  return  WIDTH_ONE_RPM / getPeriodAverage();
}



CountWinAvg	PeriodSense::winAvg[ICU_NUMBER_OF_ENTRIES];
ErrorWin	PeriodSense::winErr[ICU_NUMBER_OF_ENTRIES];
size_t		PeriodSense::indexer = 0UL;
