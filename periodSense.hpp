#pragma once
#include <ch.h>
#include <hal.h>
#include "windowsAverage.hpp"
#include "hardwareConf.hpp"

namespace Lock {
  class DiscardIsr {
    public:
    static void lock(void) {chSysSuspend();};
    static void unlock(void) {chSysEnable();};
  };
};


// window average of 6 values with higher and lower values discarded by median filter
using CountWinAvg = WindowMedianAverage<icucnt_t, 8, 1, Lock::DiscardIsr>;



class PeriodSense {
public:
  PeriodSense(void) : icup(nullptr) {};
  void		setIcu(ICUDriver * const _icup, const icuchannel_t channel);
  icucnt_t	getPeriodAverage(void) const;
  uint32_t	getRPM(void) const ;
  uint32_t	getMperiod(void) const {return winAvg[icup->index].getMean();};
  uint32_t	getRperiod(void) const {return icuGetPeriodX(icup);};
  uint32_t	getTimPsc(void) const {return icup->tim->PSC;};
  size_t	getIndex(void) const {return icup->index;}
  size_t	getDynSize(void) {return indexer;};
  
private:
  void setDivider (const uint16_t divider);


  ICUDriver *		icup;
  ICUConfig		config;
  static CountWinAvg	winAvg[TIMER_NUM_INPUT];
  static size_t		indexer;
};
