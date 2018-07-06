#pragma once
#include <ch.h>
#include <hal.h>
#include "windowsAverage.hpp"


/*
 * calcul du diviseur
 * abort si clock n'est ni sourceFreq ni 2 * sourceFreq
 * demarrage du timer
 * changement de diviseur dans la foulée
 * sur callback : nouvelle entrée dans windowAverage
 * methode : getWidthAverage


 */


using CountWinAvg = WindowAverage<icucnt_t, 6>;



class PeriodSense {
public:
  PeriodSense(ICUDriver * const _icu, const int16_t _divider,
	      const uint32_t _sourceFreq);
  uint16_t	getWidthAverage(void);
  
private:
  ICUDriver * const	icu;
  ICUConfig		config;
  const uint16_t	divider;
  const uint32_t	sourceFreq;
  CountWinAvg		winAvg;
};
