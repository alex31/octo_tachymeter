#pragma once
#include <ch.h>
#include <hal.h>
#include "windowsAverage.hpp"
#include "hardwareConf.hpp"


/*
 * demarrage du timer
 * changement de diviseur dans la foulée
 * sur callback : nouvelle entrée dans windowAverage
 * methode : getWidthAverage


 */


using CountWinAvg = WindowAverage<icucnt_t, 6>;



class PeriodSense {
public:
  PeriodSense(ICUDriver * const _icup, const uint32_t _index);
  uint16_t	getWidthAverage(void);
  uint32_t	getERPM(void);
  
private:
  void setDivider (const uint16_t divider);


  ICUDriver * const	icup;
  ICUConfig		config;
  static CountWinAvg	winAvg[TIMER_NUM_INPUT];
};
