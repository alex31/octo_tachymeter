#include <ch.h>
#include <hal.h>
#include <array>
#include "globalVar.h"
#include "stdutil.h"
#include "hardwareConf.hpp"
#include "jumperConf.hpp"
#include "rpmMsg.hpp"
#include "periodSense.hpp"
#include "messageImplChibios.hpp"
#include "userParameters.hpp"


static size_t numTrackedMotor = 0;
static std::array<PeriodSense, ICU_NUMBER_OF_ENTRIES>  psa;
static SensorType sensorType = SensorType::No_Init;

static_assert(Rpms::ASize >= ICU_NUMBER_OF_ENTRIES,
	      "Errors/Rpms array size should be >= ICU_NUMBER_OF_ENTRIES");

static THD_WORKING_AREA(waStreamer, 1024);
[[noreturn]] static void streamer (void *arg);

void rpmStartStreaming (void)
{
  messageInit();
  calcParam.cache();

  numTrackedMotor = JUMPERS.readConf(0) + 1; // [1 .. 2^^3]
  sensorType = JUMPERS.readConf(1) ?  SensorType::Esc_coupler : SensorType::Hall_effect;

  if (sensorType == SensorType::Esc_coupler) {
    while (true) {
      chThdSleepSeconds(1);
      DebugTrace ("ESC Opto Coupler sensing not yet implemented");
    }
  }
  
  for (size_t i=0; i<numTrackedMotor; i++) {
    psa[i].setIcu(ICU_TIMER[i].first, ICU_TIMER[i].second);
  }
  
  chThdCreateStatic(waStreamer, sizeof(waStreamer), NORMALPRIO, &streamer, NULL);
}

PeriodSense&    rpmGetPS(size_t index)
{
  return (index < numTrackedMotor) ? psa[index] : psa[0];
}  

uint16_t	rpmGetRPM(size_t index)
{
  return (index < numTrackedMotor) ? psa[index].getRPM() : 0;
}

size_t		rpmGetNumTrackedMotors(void)
{
  return numTrackedMotor;
}

SensorType	rpmGetSensorType(void)
{
  return sensorType;
}

/*
  template helper function to copy data from array of object via a getter method, 
  to an std:array of values in a message object
 */
template <typename F>
using ptrToMethod = uint32_t (F::*) (void) const;

template <typename T, typename F, size_t FN>
static void copyValToMsg(T& msg_s, const std::array<F, FN>& arr,
			 ptrToMethod<F> getter)
{
  for (size_t i=0; i<numTrackedMotor; i++) {
    msg_s.values[i] = (arr[i].*getter) ();
  }
  msg_s.dynSize = numTrackedMotor;
}


[[noreturn]] static void streamer (void *arg)
{
  (void) arg;
  
  chRegSetThreadName("streamer");
  Errors errors;
  Rpms   rpms;
  uint32_t cnt=0;
  systime_t ts;
  while (true) {
    ts = chVTGetSystemTimeX();
    copyValToMsg(rpms, psa, &PeriodSense::getRPM);
    FrameMsgSendObject<Msg_Rpms>::send(rpms);


    // if there is an error, message will be sent before error get out the window
    if ((cnt++ % ErrorWin::size()) == 0) {
      if (std::accumulate(psa.begin(), psa.begin()+numTrackedMotor,
			  0UL, // initialisation value of accumulator
			  [](uint32_t a, auto b) {
			    return a + b.getNumBadMeasure(); // accumulates all errors
			  })) {
	copyValToMsg(errors, psa, &PeriodSense::getNumBadMeasure);
	FrameMsgSendObject<Msg_Errors>::send(errors);
      }
      
    }
    chThdSleepUntilWindowed(ts, ts+userParam.getTicksBetweenMessages());
  }
}
