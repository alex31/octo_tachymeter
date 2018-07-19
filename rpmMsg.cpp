#include <ch.h>
#include <hal.h>
#include <array>
#include "globalVar.h"
#include "stdutil.h"
#include "hardwareConf.hpp"
#include "jumperConf.hpp"
#include "rpmMsg.hpp"
#include "simpleSerialMessage.h"
#include "periodSense.hpp"

enum class MessageId : std::uint8_t {NO_INIT=0, RPM, NUM_ERRORS};

static constexpr UARTConfig config  = {
				   .txend1_cb =nullptr,
				   .txend2_cb = nullptr,
				   .rxend_cb = nullptr,
				   .rxchar_cb = nullptr,
				   .rxerr_cb = nullptr,
				   .speed = 230400,
				   .cr1 = 0,
				   .cr2 = USART_CR2_STOP1_BITS | USART_CR2_LINEN,
				   .cr3 = 0
};

static size_t numTrackedMotor = 0;
static std::array<PeriodSense, ICU_NUMBER_OF_ENTRIES>  psa;
static SensorType sensorType = SensorType::No_Init;

template <typename T, size_t N, MessageId ID>
struct StreamMessage {
  constexpr StreamMessage(size_t len) : mid{ID}, hpload{0}, values{0}, vlen{len},
					vsize{(sizeof(T) * len) + sizeof(mid) + sizeof(hpload)}
					{}; 
  size_t size(void) const {return vsize;};
  size_t len(void) const {return vlen;};
  const uint8_t* data() const {return reinterpret_cast<const uint8_t *>(this);};
  const enum MessageId mid;
  uint8_t hpload[3];
  std::array<T, N>     values;
  const size_t vlen;
  const size_t vsize;
} __attribute__ ((__packed__)) ;


static THD_WORKING_AREA(waStreamer, 1024);
[[noreturn]] static void streamer (void *arg);

void rpmStartStreaming (void)
{
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
  
  uartStart(&UARTD4, &config);
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


template <typename F>
using ptrToMethod = uint32_t (F::*) (void) const;

template <typename T, size_t N, MessageId ID, typename F, size_t FN>
void copyMsg(StreamMessage<T, N, ID>& streamMsg, const std::array<F, FN>& arr, ptrToMethod<F> getter)
{
  for (size_t i=0; i<streamMsg.len(); i++) {
    streamMsg.values[i] = (arr[i].*getter) ();
  }
}


[[noreturn]] static void streamer (void *arg)
{
  (void) arg;
  
  chRegSetThreadName("streamer");
  StreamMessage<uint16_t, ICU_NUMBER_OF_ENTRIES, MessageId::RPM> rpmMessage(numTrackedMotor);
  StreamMessage<uint8_t, ICU_NUMBER_OF_ENTRIES, MessageId::NUM_ERRORS> errMessage(numTrackedMotor);

  uint32_t cnt=0;
  while (true) {
    copyMsg(rpmMessage, psa, &PeriodSense::getRPM);
    if (not simpleMsgSend (&UARTD4, rpmMessage.data(),
			   rpmMessage.size())) {
	DebugTrace ("simpleMsgSend RPM has failed");
      }
    
    if ((cnt++ % 64) == 0) {
      bool badCond=false;
      for (size_t i=0; i<numTrackedMotor; i++) {
	if (psa[i].getNumBadMeasure()) {
	  badCond=true;
	  break;
	}
      }

      if (badCond) {
	copyMsg(errMessage, psa, &PeriodSense::getNumBadMeasure);
	if (not simpleMsgSend (&UARTD4, errMessage.data(),
			       errMessage.size())) {
	  DebugTrace ("simpleMsgSend ERR has failed");
	}
      }
      
    }
    //    chThdSleepSeconds(1);
  }
}
