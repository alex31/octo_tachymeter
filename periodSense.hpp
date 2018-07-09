#pragma once
#include <ch.h>
#include <hal.h>
#include "windowsAverage.hpp"
#include "hardwareConf.hpp"


// window average of 6 values with higher and lower values discarded by median filter
using CountWinAvg = WindowMedianAverage<icucnt_t, 8, 1>;



class PeriodSense {
public:
  PeriodSense(ICUDriver * const _icup, const icuchannel_t channel);
  uint16_t	getPeriodAverage(void) const;
  uint32_t	getRPM(void) const ;
  uint32_t	getMperiod(void) const {return winAvg[icup->index].getMean(true);};
  uint32_t	getTimPsc(void) const {return icup->tim->PSC;};
  size_t	getIndex(void) const {return icup->index;}
  
private:
  void setDivider (const uint16_t divider);


  ICUDriver * const	icup;
  ICUConfig		config;
  static CountWinAvg	winAvg[TIMER_NUM_INPUT];
  static size_t		indexer;
};
