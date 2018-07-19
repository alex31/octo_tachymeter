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

struct StreamMessage {
  StreamMessage(const enum MessageId id) : mid{id}, payloadSize{getPayloadSize(id)} {};
  static  size_t getPayloadSize(const enum MessageId id) {
    switch (id) {
    case MessageId::NO_INIT : return 0;
    case MessageId::RPM : return sizeof(rpmValues) + sizeof(mid);
    case MessageId::NUM_ERRORS : return sizeof(errValues) + sizeof(mid);
    default: return 0;
    };
  };
  union {
    std::array<uint16_t, ICU_NUMBER_OF_ENTRIES>     rpmValues;
    std::array<uint8_t, ICU_NUMBER_OF_ENTRIES>      errValues;
  };
  const enum MessageId mid;
  const size_t payloadSize;
};

static StreamMessage rpmMessage(MessageId::RPM);
static StreamMessage errMessage(MessageId::NUM_ERRORS);

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
  
  while (true) {
    for (size_t i=0; i<numTrackedMotor; i++) {
      rpmMessage.rpmValues[i] = psa[i].getRPM();
    }

    simpleMsgSend (&UARTD4, reinterpret_cast<uint8_t *>(rpmMessage.rpmValues.data()),
		   rpmMessage.payloadSize);
  }
}
