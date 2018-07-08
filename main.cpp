#include <ch.h>
#include <hal.h>
#include "globalVar.h"
#include "stdutil.h"
#include "ttyConsole.h"
#include "led_blink.hpp"
#include "hardwareConf.hpp"
#include "periodSense.hpp"
#include "pwm.h"
/*
  Câbler une LED sur la broche C0

Câbler sur la carte de dev le chip convertisseur USB série :

ftdi RX sur B6 (utiliser un jumper)
ftdi TX sur B7 (utiliser un jumper)

connecter PA5 sur PA2 et PC6

*/

/*

  TODO : 
  * tester avec un capteur à effet hall
  * tester avec plusieurs entrées
  * ajouter un filtre median
  * tester la validité de la mesure en utilisant width car normalement (tester avant)
    width doit être approximativement égal à period / 2
  * deux types de messages : rpm et qualité
  * tester envoi sur UART par DMA
 */


static THD_WORKING_AREA(waBlinker, 256);
[[noreturn]] static void blinker (void *arg)
{
  (void)arg;
  chRegSetThreadName("blinker");
  PeriodSense ps[]{&ICUD12} ;
 
  while (true) { 
    chThdSleepMilliseconds (500);
    //DebugTrace ("rpm = %lu w=%lu", ps[0].getRPM(), icuGetPeriodX(&ICUD8));
    DebugTrace ("rpm = %lu w=%lu f=%lu", ps[0].getRPM(), ps[0].getMperiod(0), pwmGetFreq());
    // DebugTrace ("FREQ_AT_MAX_RPM=%lu  FREQ_AT_MIN_RPM=%lu "
    // 		"TICK_AT_MIN_RPM=%lu TIM_DIVIDER=%lu "
    // 		"NB_TICKS_AT_MAX_RPM=%lu",
    // 		FREQ_AT_MAX_RPM, FREQ_AT_MIN_RPM, TICK_AT_MIN_RPM,
    // 		TIM_DIVIDER, NB_TICKS_AT_MAX_RPM);
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
  chThdSleepSeconds(1);
  ledBlink.setFlashes(2, 4);
  launchPwm();
  
  // main thread does nothing
  chThdSleep (TIME_INFINITE);
}

