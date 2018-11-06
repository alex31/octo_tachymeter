#include <ch.h>
#include <hal.h>
#include <array>
#include "globalVar.h"
#include "stdutil.h"
#include "hardwareConf.hpp"
#include "rpmMsg.hpp"
#include "periodSense.hpp"
#include "userParameters.hpp"
#include "messageImplChibios.hpp"


using IcuEntry = std::pair<ICUDriver * const, const icuchannel_t>;

static constexpr std::array<IcuEntry, ICU_NUMBER_OF_ENTRIES> ICU_TIMER = {{
      {&ICUD1, ICU_CHANNEL_1},  // 168
#if USE_TIM2_IN_PWM_MODE_FOR_SELF_TESTS == 0
      {&ICUD2, ICU_CHANNEL_1},  // 84
#endif
      {&ICUD3, ICU_CHANNEL_1},  // 84
      {&ICUD4, ICU_CHANNEL_1},  // 84
      {&ICUD5, ICU_CHANNEL_1},  // 84
      {&ICUD8, ICU_CHANNEL_1},  // 168
      {&ICUD9, ICU_CHANNEL_1},  // 168
      {&ICUD12, ICU_CHANNEL_1}, // 168
    }} ;

//static size_t numTrackedMotor = 0;
static std::array<PeriodSense, ICU_NUMBER_OF_ENTRIES>  psa;
//static SensorType sensorType = SensorType::No_Init;
static thread_t	  *streamerThd = nullptr;

static_assert(Rpms::ASize >= ICU_NUMBER_OF_ENTRIES,
	      "Errors/Rpms array size should be >= ICU_NUMBER_OF_ENTRIES");

static THD_WORKING_AREA(waStreamer, 1024);
static void hallSensorStreamer (void *arg);

void rpmStartStreaming (void)
{
  //  numTrackedMotor = JUMPERS.readConf(0) + 1; // [1 .. 2^^3]
  //  sensorType = JUMPERS.readConf(1) ?  SensorType::Esc_coupler : SensorType::Hall_effect;
  if (streamerThd != nullptr) {
    FrameMsgSendObject<Msg_TachoError>::send(TachoError("rpmStartStreaming error : streamerThd != null"));
    return;
  }
  userParam.setRunningState(RunningState::Run);
  calcParam.cache();
  
  if (userParam.getSensorType() == SensorType::Esc_coupler) {
    FrameMsgSendObject<Msg_TachoError>::send(TachoError("ESC Opto Coupler sensing not yet implemented"));
    DebugTrace ("ESC Opto Coupler sensing not yet implemented");
  } else {
    streamerThd = chThdCreateStatic(waStreamer, sizeof(waStreamer),
				    NORMALPRIO, &hallSensorStreamer, NULL);
  }
}

void rpmStopStreaming (void)
{
  if (streamerThd != nullptr) {
    chThdTerminate(streamerThd);
    chThdWait(streamerThd);
    streamerThd = nullptr;
    userParam.setRunningState(RunningState::Stop);
  }
}


PeriodSense&    rpmGetPS(size_t index)
{
  return (index < userParam.getNbMotors()) ? psa[index] : psa[0];
}  

uint16_t	rpmGetRPM(size_t index)
{
  return (index < userParam.getNbMotors()) ? psa[index].getRPM() : 0;
}

size_t		rpmGetNumTrackedMotors(void)
{
  return userParam.getNbMotors();
}

SensorType	rpmGetSensorType(void)
{
  return userParam.getSensorType();
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
  for (size_t i=0; i<userParam.getNbMotors(); i++) {
    msg_s.values[i] = (arr[i].*getter) ();
  }
  msg_s.dynSize = userParam.getNbMotors();
}


static void hallSensorStreamer (void *arg)
{
  (void) arg;
  
  chRegSetThreadName("hallSensorStreamer");
  calcParam.cache();

  Errors errors;
  Rpms   rpms;
  uint32_t cnt=0;
  systime_t ts;

  for (size_t i=0; i<userParam.getNbMotors(); i++) {
    psa[i].setIcu(ICU_TIMER[i].first, ICU_TIMER[i].second);
  }
  
  while (not chThdShouldTerminateX()) {
    ts = chVTGetSystemTimeX();
    copyValToMsg(rpms, psa, &PeriodSense::getRPM);
    FrameMsgSendObject<Msg_Rpms>::send(rpms);


    // if there is an error, message will be sent before error get out the window
    if ((cnt++ % ErrorWin::size()) == 0) {
      if (std::accumulate(psa.begin(), psa.begin()+userParam.getNbMotors(),
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
  for (size_t i=0; i<userParam.getNbMotors(); i++) {
    psa[i].stopIcu();
  }
  PeriodSense::resetIcu();

  chThdExit(0);
}
