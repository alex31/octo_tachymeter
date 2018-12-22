#include <ch.h>
#include <hal.h>
#include "array"
#include "globalVar.h"
#include "stdutil.h"
#include "ttyConsole.hpp"
#include "led_blink.hpp"
#include "hardwareConf.hpp"
#include "periodSense.hpp"
#include "pwm.h"
#include "rpmMsg.hpp"
#include "userParameters.hpp"
#include "messageImplChibios.hpp"

/*
Connecter sur la carte de dev le chip convertisseur USB série  :
  ftdi RX sur B10 (enlever le jumper)
  ftdi TX sur B11 (enlever le jumper)

  Connecter une LED sur la broche C2 (flash led)
  Connecter PA5 sur PA2, PB14, PC6

*/

/*

  TODO :
 
  * sauvegarde en flash des paramètres du filtre passe bas + median

  * fix mode OPTO : debugger avec l'analyseur logique pour voir si le pb 
    à la décélération est solutionable

  * entrée sensor et sensorless sur deux broches differentes 
    pour chacun des 8 timers

  * portage carte devboard_M767

  * hardware dédié à base de F722

  * portage hardware dédié

 */


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

  userParam.readConfFromEEprom();
  messageInit();

  if (userParam.getRunningState() == RunningState::Run) {
    rpmStartStreaming();
  } 

  
  
  ledBlink.setFlashes(rpmGetNumTrackedMotors(),
		      rpmGetSensorType() == SensorType::Hall_effect ? 1 : 2);

#if  USE_TIM2_IN_PWM_MODE_FOR_SELF_TESTS
  launchPwm();
#endif
  
  // main thread does nothing
  chThdSleep(TIME_INFINITE);
}

