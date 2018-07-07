#pragma once
#include <ch.h>
#include <hal.h>
#include "windowsAverage.hpp"
#include "hardwareConf.hpp"


/*
 * demarrage du timer
 * changement de diviseur dans la foulée
 * sur callback : nouvelle entrée dans windowAverage
 * methode : getPeriodAverage


 */


using CountWinAvg = WindowAverage<icucnt_t, 6>;



class PeriodSense {
public:
  PeriodSense(ICUDriver * const _icup);
  uint16_t	getPeriodAverage(void);
  uint32_t	getERPM(void);
  uint32_t	getMperiod(const size_t idx){return winAvg[idx].getMean();};
  
private:
  void setDivider (const uint16_t divider);


  ICUDriver * const	icup;
  ICUConfig		config;
  static CountWinAvg	winAvg[TIMER_NUM_INPUT];
  static size_t		indexer;
};
