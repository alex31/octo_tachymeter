#include <ch.h>
#include <hal.h>
#include "periodSense.hpp"
#include "hardwareConf.hpp"
#include <climits>

PeriodSense::PeriodSense(ICUDriver * const _icu, const int16_t _divider,
			 const uint32_t _sourceFreq):
  icu(_icu), divider(_divider), sourceFreq(_sourceFreq)
{
  osalDbgAssert ((icu->clock == sourceFreq) || (icu->clock == (2UL * sourceFreq)),
		 "sourceFreq not compatible with timer source clock");

  config = ICUConfig {
    .mode = ICU_INPUT_ACTIVE_HIGH,
    .frequency = 1_mhz,              /* real divider will be calculated dynamically  */
    .width_cb = nullptr,
    .period_cb = nullptr,
    .overflow_cb =  [] (ICUDriver *licu) {licu->hasOverflow = true;} ,
    .channel = ICU_CHANNEL_1,
    .dier = 0
  };

  icu->hasOverflow = false;
  //icu->callable =  [] (void *ps) {osalDbgAssert(ps == nullptr, "test");};
  icu->callable =  [] (void *_ps) {
    PeriodSense *ps = (PeriodSense *) _ps;
    ps->winAvg.push(icuGetWidthX(ps->icu));
  };

}

uint16_t	PeriodSense::getWidthAverage(void) {
  if (icu->hasOverflow) {
    return USHRT_MAX;
    icu->hasOverflow = false;
  } else {
    return winAvg.getMean();
  }
};
