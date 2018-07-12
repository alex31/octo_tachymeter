#include <ch.h>
#include <hal.h>
#include <array>
#include "hardwareConf.hpp"
#include "rpmMsg.hpp"
#include "simpleSerialMessage.h"

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


void startRpmStreaming(void)
{
  std::array<uint16_t, 8> rpmValues = {1,2,3,4,5,6,7,8};
  uartStart(&UARTD4, &config);

  while (true) {
    simpleMsgSend (&UARTD4, reinterpret_cast<uint8_t *>(rpmValues.data()),
		   rpmValues.size() * (sizeof(rpmValues[0] / sizeof(uint8_t))));
    //    chThdSleepMilliseconds(1);
  }
}
