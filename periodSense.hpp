#pragma once
#include <ch.h>
#include <hal.h>
#include "windowsAverage.hpp"
#include "hardwareConf.hpp"

#define ICU_NUMBER_OF_ENTRIES 8



namespace Lock {
  class DiscardIsr {
    public:
    static inline void lock(void) {chSysSuspend();};
    static inline void unlock(void) {chSysEnable();};
  };
};


// window average of 6 values with higher and lower values discarded by median filter
using CountWinAvg = ResizableWindowMedianAverage<icucnt_t, INIT_WINDOW_FILTER_SIZE, Lock::DiscardIsr>;
using ErrorWin =    WindowAverage<uint8_t, INIT_ERRORS_WINDOW_SIZE>;



class PeriodSense {
public:
  PeriodSense() : icup(nullptr) {};
  void		 setIcu(ICUDriver * const _icup, const icuchannel_t channel);
  void		 stopIcu(void);
  static void	 resetIcu(void) {indexer=0U;};
  static size_t	 getDynSize(void) {return indexer;};
  static void	 setWinAvgSize(const uint8_t size);
  static uint8_t getWinAvgSize(void);
  static void	 setWinAvgMedianSize(const uint8_t size);
  static uint8_t getWinAvgMedianSize(void);
  icucnt_t	 getPeriodAverage(void) const;
  uint32_t	 getRPM(void) const ;
  uint32_t	 getMperiod(void) const {return winAvg[icup->index].getMean();};
  uint32_t	 getNumBadMeasure(void) const {return winErr[icup->index].getSum();};
  uint32_t	 getRperiod(void) const {return icuGetPeriodX(icup);};
  uint32_t	 getRWidth(void) const {return icuGetWidthX(icup);};
  uint32_t	 getTimPsc(void) const {return icup->tim->PSC;};
  size_t	 getIndex(void) const {return icup->index;}
  void		 setWidthOneRpm(const uint32_t w) {widthOneRpm=w;};
  uint32_t	 getWidthOneRpm(void) const {return widthOneRpm;};
  
private:
  void setDivider (const uint16_t divider);
  void setIcuForHallSensor(ICUDriver * const _icup, const icuchannel_t channel);
#if OPTOCOUPLER_ON_BOARD
  void setIcuForOptoCouplerSensor(ICUDriver * const _icup, const icuchannel_t channel);
#endif

  ICUDriver *		icup;
  ICUConfig		config;
  uint32_t		widthOneRpm;

  static CountWinAvg	winAvg[ICU_NUMBER_OF_ENTRIES];
  static ErrorWin	winErr[ICU_NUMBER_OF_ENTRIES];
#if OPTOCOUPLER_ON_BOARD
  static uint32_t	optoTimeStamp[ICU_NUMBER_OF_ENTRIES];
#endif
  static size_t		indexer;
  
};
