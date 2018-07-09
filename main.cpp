#include <ch.h>
#include <hal.h>
#include "array"
#include "globalVar.h"
#include "stdutil.h"
#include "ttyConsole.h"
#include "led_blink.hpp"
#include "hardwareConf.hpp"
#include "periodSense.hpp"
#include "pwm.h"
/*
Connecter sur la carte de dev le chip convertisseur USB série  :
  ftdi RX sur B10 (enlever le jumper)
  ftdi TX sur B11 (enlever le jumper)

  Connecter une LED sur la broche C0 (overflow)
  Connecter le potar à bouton sur la broche  C1 (overflow)
  Connecter une LED sur la broche C2 (flash led)
  Connecter PA5 sur PA2, PB14, PC6

*/

/*

  TODO : 
  * tester avec un capteur à effet hall
  * tester la validité de la mesure en utilisant width car normalement (tester avant)
    width doit être approximativement égal à period / 2
  * deux types de messages : rpm et qualité
  * tester envoi sur UART par DMA
 */

volatile uint32_t dbgRes;

static THD_WORKING_AREA(waBlinker, 1024);
[[noreturn]] static void blinker (void *arg)
{
  (void)arg;
  chRegSetThreadName("blinker");
  std::array<PeriodSense, 7> psa = {{
      {&ICUD1, ICU_CHANNEL_1},  // 168
#ifndef USE_TIM2_IN_PWM_MODE_FOR_SELF_TESTS
      {&ICUD2, ICU_CHANNEL_1},  // 84
#endif
      {&ICUD3, ICU_CHANNEL_1},  // 84
      {&ICUD4, ICU_CHANNEL_1},  // 84
      {&ICUD5, ICU_CHANNEL_1},  // 84
      {&ICUD8, ICU_CHANNEL_1},  // 168
      {&ICUD9, ICU_CHANNEL_1},  // 168
      {&ICUD12, ICU_CHANNEL_1}, // 168
    }} ;
 
  while (true) { 
    chThdSleepMilliseconds (1000);
    for (int i=0; i<1000; i++) {
      for (const PeriodSense &ps : psa) {
	dbgRes = ps.getRPM();
      }
    }

    //DebugTrace ("rpm = %lu w=%lu", ps[0].getRPM(), icuGetPeriodX(&ICUD8));
    for (const PeriodSense &ps : psa) {
      DebugTrace ("rpm[%u] = %lu w=%lu psc=%lu f=%lu", ps.getIndex(),
		  ps.getRPM(), ps.getMperiod(), ps.getTimPsc(), pwmGetFreq());
    }
    DebugTrace("-----------------------");
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

