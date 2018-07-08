#pragma once
#include <ch.h>
#include <hal.h>
#include "windowsAverage.hpp"
#include "hardwareConf.hpp"


// window average of 6 values with higher and lower values discarded by median filter
using CountWinAvg = WindowMedianAverage<icucnt_t, 8, 1>;



class PeriodSense {
public:
  PeriodSense(ICUDriver * const _icup);
  uint16_t	getPeriodAverage(void);
  uint32_t	getRPM(void);
  uint32_t	getMperiod(const size_t idx){return winAvg[idx].getMean();};
  
private:
  void setDivider (const uint16_t divider);


  ICUDriver * const	icup;
  ICUConfig		config;
  static CountWinAvg	winAvg[TIMER_NUM_INPUT];
  static size_t		indexer;
};
