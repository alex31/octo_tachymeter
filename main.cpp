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
#include "userParameters.hpp"
#include "jumperConf.hpp"
#include "messageImplChibios.hpp"

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
  * tester la validité de la mesure en utilisant width car normalement (tester avant)
    width doit être approximativement égal à period / 2

  * alternative au capteur effet hall : interface via isolation galvanique 
    sur une sortie controleur moteur sur entrée timer (la même que pour le pwm)
    ° soit sous interruption (attention à l'avalanche d'IT possible)
      50khz*8 = 400.000 IT/seconde => pas plus de 100 cycles d'horloge par IT, l'algo doit être
      simplissime
    ° soit en dma (on gagne les cycles d'entrée sortie de l'IT) 

  * portage carte devboard_M767

  * hardware dédié à base de F722

  * portage hardware dédié

 */

// all this if for tuning, will not be compiled in production code
#ifdef TRACE
static THD_WORKING_AREA(waBlinker, 1024);
[[noreturn]] static void blinker (void *arg)
{
  (void)arg;
  chRegSetThreadName("blinker");

  const size_t numTrackedMotor = rpmGetNumTrackedMotors();

   
 
  while (true) { 
    chThdSleepMilliseconds (1000);
    //DebugTrace ("rpm = %lu w=%lu", ps[0].getRPM(), icuGetPeriodX(&ICUD8));
    for (size_t i=0; i<numTrackedMotor; i++) {
      const PeriodSense &ps = rpmGetPS(i);
#if USE_TIM2_IN_PWM_MODE_FOR_SELF_TESTS      
      DebugTrace ("TEST: rpm[%u] = %lu rp=%lu ap=%lu psc=%lu f=%lu", ps.getIndex(),
		  ps.getRPM(), ps.getRperiod(), ps.getMperiod(), ps.getTimPsc(), pwmGetFreq());
#else
          DebugTrace ("rpm[%u] = %lu rp=%lu ap=%lu psc=%lu P/W=%.3f Err=%lu", ps.getIndex(),
		      ps.getRPM(), ps.getRperiod(), ps.getMperiod(), ps.getTimPsc(),
		      static_cast<float>(ps.getRperiod()) / ps.getRWidth(),
		      ps.getNumBadMeasure());
#endif      
    }
    DebugTrace("-----------------------");
    // DebugTrace ("FREQ_AT_MAX_RPM=%lu  FREQ_AT_MIN_RPM=%lu "
    // 		"TICK_AT_MIN_RPM=%lu TIM_DIVIDER=%lu "
    // 		"NB_TICKS_AT_MAX_RPM=%lu",
    // 		FREQ_AT_MAX_RPM, FREQ_AT_MIN_RPM, TICK_AT_MIN_RPM,
    // 		TIM_DIVIDER, NB_TICKS_AT_MAX_RPM);
  }
}
#endif

void _init_chibios() __attribute__ ((constructor(101)));
void _init_chibios() {
  halInit();
  chSysInit();
  initHeap();
}




int main(void)
{

    /*
   * System initializations.
   * - HAL initialization, this also initializes the configured device drivers
   *   and performs the board-specific initializations.
   * - Kernel initialization, the main() function becomes a thread and the
   *   RTOS is active.
   */

#ifdef TRACE
  consoleInit();
  consoleLaunch();
#endif

  messageInit();
  userParam.setNbMotors(JUMPERS.readConf(0) + 1);
  userParam.setSensorType(JUMPERS.readConf(1) ?
			  SensorType::Esc_coupler : SensorType::Hall_effect);
  if (INIT_RUNNING_STATE == RunningState::Run) {
    rpmStartStreaming();
  } else {
    userParam.setRunningState(RunningState::Stop);
  }
  
#ifdef TRACE
  chThdCreateStatic(waBlinker, sizeof(waBlinker), NORMALPRIO, &blinker, NULL);
  
  DebugTrace("config nb motors  = %u", rpmGetNumTrackedMotors());
  DebugTrace("config sensor mode = %s",
	     rpmGetSensorType() == SensorType::Esc_coupler ? "ESC Opto Coupler" :
	     "Hall Effect Sensor");
#endif
  
  ledBlink.setFlashes(rpmGetNumTrackedMotors(),
		      rpmGetSensorType() == SensorType::Hall_effect ? 1 : 2);
#if  USE_TIM2_IN_PWM_MODE_FOR_SELF_TESTS
  launchPwm();
#endif
  
  // main thread does nothing
  chThdSleep(TIME_INFINITE);
}

