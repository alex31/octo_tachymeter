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
  constexpr StreamMessage(void) : mid{ID}, hpload{0}, values{0} {}; 
  constexpr size_t sizeOf(size_t numTracked) {
    return (sizeof(T) * numTracked) + sizeof(mid) + sizeof(hpload);
  };
  uint8_t* data() {return reinterpret_cast<uint8_t *>(this);};
  const enum MessageId mid;
  uint8_t hpload[3];
  std::array<T, N>     values;
} __attribute__ ((__packed__)) ;

static StreamMessage<uint16_t, ICU_NUMBER_OF_ENTRIES, MessageId::RPM> rpmMessage;
static StreamMessage<uint8_t, ICU_NUMBER_OF_ENTRIES, MessageId::NUM_ERRORS>  errMessage;

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

[[noreturn]] static void streamer (void *arg)
{
  (void) arg;
  
  chRegSetThreadName("streamer");
  uint32_t cnt=0;
  while (true) {
    for (size_t i=0; i<numTrackedMotor; i++) {
      rpmMessage.values[i] = psa[i].getRPM();
    }
    if (not simpleMsgSend (&UARTD4, rpmMessage.data(),
			   rpmMessage.sizeOf(numTrackedMotor))) {
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
	for (size_t i=0; i<numTrackedMotor; i++) {
	  errMessage.values[i] = psa[i].getNumBadMeasure();
	}
	if (not simpleMsgSend (&UARTD4, errMessage.data(),
			       errMessage.sizeOf(numTrackedMotor))) {
	  DebugTrace ("simpleMsgSend ERR has failed");
	}
      }
      
    }
    //    chThdSleepSeconds(1);
  }
}
