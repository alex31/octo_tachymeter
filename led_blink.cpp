#include <ch.h>
#include <hal.h>
#include "stdutil.h"
#include "led_blink.hpp"
#ifdef TRACE
#include <strings.h>
#endif

LedBlink ledBlink(LINE_FLASH_LED, 2000, 600, 200);

uint8_t LedBlink::indexer = 0;

void  LedBlink::launchThread(void)
{
  chThdCreateStatic(waBlinker, sizeof(waBlinker), NORMALPRIO,
		     []  (void *arg) [[noreturn]] {
		      const LedBlink *led = static_cast<LedBlink *>(arg);
		      led->thdBlinkLed();
		    },
		    (void *) this);
}


[[noreturn]] void LedBlink::thdBlinkLed (void) const
{
  //  const LedBlink *ledBlink = (LedBlink *) arg;

  // give uniq name to threads if several leds are used
#ifdef TRACE
  *(index(name, '#')) = indexer++;
  chRegSetThreadName (name);
#endif
  
  while (true) {
    for (uint32_t i=0; i< seq1Flashes; i++) {
      palToggleLine(ledLine);
      chThdSleepMilliseconds(msBetweenFlashes);
    }
    palClearLine(ledLine);

    if (msBetweenSeq1Seq2) {
      chThdSleepMilliseconds(msBetweenSeq1Seq2);

      for (uint32_t i=0; i< seq2Flashes; i++) {
	palToggleLine(ledLine);
	chThdSleepMilliseconds(msBetweenFlashes);
      }
      palClearLine(ledLine);
    }
    
    chThdSleepMilliseconds(msBetweenSeqs);
  }
  
}
