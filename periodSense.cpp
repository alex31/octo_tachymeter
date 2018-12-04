#include <ch.h>
#include <hal.h>
#include "periodSense.hpp"
#include "userParameters.hpp"
#include "messageImplChibios.hpp"

#include <climits>

void PeriodSense::setIcuForHallSensor(ICUDriver * const _icup, const icuchannel_t channel)
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
      //      palClearLine(LINE_LED1);
#endif
    } ,
    .overflow_cb =  [] (ICUDriver *licup) {
      winErr[licup->index].push(1);
      licup->hasOverflow = true;
#ifdef TRACE
      //      palSetLine(LINE_LED1); 	
#endif
    },
    .channel = channel,
    .dier = 0
  };

  icup->hasOverflow = false;

  icuStart(icup, &config);
  
  osalDbgAssert((icup->clock ==  TIMER_FREQ_IN) || (icup->clock == (2UL * TIMER_FREQ_IN)),
		 "TIMER_FREQ_IN not compatible with timer source clock");

  setDivider(calcParam.getTimDivider());
  setWidthOneRpm(calcParam.getWidthOneRpmHall());  
  icuStartCapture(icup);
  icuEnableNotifications(icup);
}

#if OPTOCOUPLER_ON_BOARD 
void PeriodSense::setIcuForOptoCouplerSensor(ICUDriver * const _icup, const icuchannel_t channel)
{
  osalDbgAssert((indexer < ICU_NUMBER_OF_ENTRIES),
		"not enough index in array, modify ICU_NUMBER_OF_ENTRIES in hardwareConf.hpp");

  icup = _icup;
  icup->index = indexer++;
  
  config = ICUConfig {
    .mode = ICU_INPUT_ACTIVE_LOW,
    .frequency = TIMER_FREQ_OPTO,
    .width_cb = nullptr,
    .period_cb = [] (ICUDriver *licup) {
      const uint32_t p = icuGetPeriodX(licup);
      const uint32_t w = icuGetWidthX(licup);
      const uint32_t inactive = p - w;
      if (inactive > INACTIVE_DURATION_TO_DETECT_END) {
	const uint32_t nowTS = chSysGetRealtimeCounterX();
	const uint32_t oldTS = std::exchange(optoTimeStamp[licup->index], nowTS);
	const uint32_t diff = (nowTS - oldTS);
	winAvg[licup->index].push(diff);
	winErr[licup->index].push(0);
      }
      licup->hasOverflow = false;
#ifdef TRACE
      //      palClearLine(LINE_LED1);
#endif
    } ,
    .overflow_cb =  [] (ICUDriver *licup) {
      winErr[licup->index].push(1);
      licup->hasOverflow = true;
#ifdef TRACE
      //      palSetLine(LINE_LED1); 	
#endif
    },
    .channel = channel,
    .dier = 0
  };
  setWidthOneRpm(calcParam.getWidthOneRpmOpto());
  icup->hasOverflow = false;

  icuStart(icup, &config);
  icuStartCapture(icup);
  icuEnableNotifications(icup);
}
#endif

void PeriodSense::setIcu(ICUDriver * const _icup, const icuchannel_t channel)
{
#ifdef TRACE
  if (userParam.getInterleavedSensor() == true) {
    if ((indexer %2) == 0) {
      setIcuForHallSensor(_icup, channel);
    } else {
#if OPTOCOUPLER_ON_BOARD 
      setIcuForOptoCouplerSensor(_icup, channel);
#endif
    }
  } else {
    switch (userParam.getSensorType()) {
#if OPTOCOUPLER_ON_BOARD    
    case  SensorType::Esc_coupler :
      setIcuForOptoCouplerSensor(_icup, channel);
      break;
#endif   
    case  SensorType::Hall_effect :
      setIcuForHallSensor(_icup, channel);
      break;
    default:
      FrameMsgSendObject<Msg_TachoError>::send(TachoError("err: invalid SensorType value"));
    }
  }
#else // TRACE
  switch (userParam.getSensorType()) {
#if OPTOCOUPLER_ON_BOARD  
  case  SensorType::Esc_coupler :
    setIcuForOptoCouplerSensor(_icup, channel);
    break;
#endif
  case  SensorType::Hall_effect :
    setIcuForHallSensor(_icup, channel);
    break;
  default:
    FrameMsgSendObject<Msg_TachoError>::send(TachoError("err: invalid SensorType value"));
  }
#endif
}

  void PeriodSense::stopIcu(void)
{
  icuDisableNotifications(icup);
  icuStopCapture(icup);
  icuStop(icup);
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
  return  getWidthOneRpm() / getPeriodAverage();
}



CountWinAvg	PeriodSense::winAvg[ICU_NUMBER_OF_ENTRIES];
ErrorWin	PeriodSense::winErr[ICU_NUMBER_OF_ENTRIES];
#if OPTOCOUPLER_ON_BOARD 
uint32_t	PeriodSense::optoTimeStamp[ICU_NUMBER_OF_ENTRIES] = {0};
#endif
size_t		PeriodSense::indexer = 0UL;

