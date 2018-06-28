#include <ch.h>
#include <hal.h>
#include "globalVar.h"
#include "stdutil.h"
#include "ttyConsole.h"
#include "led_blink.hpp"

/*
  Câbler une LED sur la broche C0

Câbler sur la carte de dev le chip convertisseur USB série :

ftdi RX sur B6 (utiliser un jumper)
ftdi TX sur B7 (utiliser un jumper)

*/




static THD_WORKING_AREA(waBlinker, 256);
[[noreturn]] static void blinker (void *arg)
{
  (void)arg;
  chRegSetThreadName("blinker");
  
  while (TRUE) { 
    palToggleLine (LINE_C00_LED1); 	
    chThdSleepMilliseconds (500);
  }
}

void _init_chibios() __attribute__ ((constructor(101)));
void _init_chibios() {
  halInit();
  chSysInit();
  initHeap();
}


int main(void) {

    /*
   * System initializations.
   * - HAL initialization, this also initializes the configured device drivers
   *   and performs the board-specific initializations.
   * - Kernel initialization, the main() function becomes a thread and the
   *   RTOS is active.
   */
  
  consoleInit();
  chThdCreateStatic(waBlinker, sizeof(waBlinker), NORMALPRIO, &blinker, NULL);

  consoleLaunch();
  chThdSleepSeconds(3);
  ledBlink.setFlashes(2, 4);

  // main thread does nothing
  chThdSleep (TIME_INFINITE);
}

void  port_halt (void)
{
  while (1) ;
}

void generalKernelErrorCB (void)
{
  port_halt ();
}
