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
#include "rpmMsg.hpp"
#include "jumperConf.hpp"
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
  * coder envoi sur UART par DMA
  * tester envoi sur UART par DMA

  * tester avec un capteur à effet hall

  * tester la validité de la mesure en utilisant width car normalement (tester avant)
    width doit être approximativement égal à period / 2

  * deux types de messages : rpm et qualité

  * alternative au capteur effet hall : interface via isolation galvanique 
    sur une sortie controleur moteur : scanner un port en DMA (cadencé par timer)

  * utiliser 4 broches (resistance zero ohm ou pont de soudure) pour coder :
    + le type d'entrée (HALL ou ESC) : 1 bit
    + le nombre de moteurs :  3 bits
 */

volatile uint32_t dbgRes;

static THD_WORKING_AREA(waBlinker, 1024);
[[noreturn]] static void blinker (void *arg)
{
  (void)arg;
  chRegSetThreadName("blinker");
  const size_t numTrackedMotor = JUMPERS.readConf(0);

  PeriodSense psa[numTrackedMotor];
  for (size_t i=0; i<numTrackedMotor; i++) {
    psa[i].setIcu(ICU_TIMER[i].first, ICU_TIMER[i].second);
  }
  
 
  while (true) { 
    chThdSleepMilliseconds (1000);
    for (int i=0; i<1000; i++) {
      for (size_t j=0; j<numTrackedMotor; j++) {
	dbgRes = psa[j].getRPM();
      }
    }

    //DebugTrace ("rpm = %lu w=%lu", ps[0].getRPM(), icuGetPeriodX(&ICUD8));
    for (size_t i=0; i<numTrackedMotor; i++) {
      const PeriodSense &ps = psa[i];
      DebugTrace ("rpm[%u] = %lu rp=%lu ap=%lu psc=%lu f=%lu", ps.getIndex(),
		  ps.getRPM(), ps.getRperiod(), ps.getMperiod(), ps.getTimPsc(), pwmGetFreq());
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
  DebugTrace("config nb entries  = %lu", JUMPERS.readConf(0));
  DebugTrace("config sensor mode = %lu", JUMPERS.readConf(1));
  chThdSleepSeconds(1);
  ledBlink.setFlashes(2, 4);
#ifdef  USE_TIM2_IN_PWM_MODE_FOR_SELF_TESTS
  launchPwm();
#endif
  
  startRpmStreaming();
  // main thread does nothing
  chThdSleep(TIME_INFINITE);
}

