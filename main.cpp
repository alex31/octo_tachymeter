#include <ch.h>
#include <hal.h>
#include "array"
#include "globalVar.h"
#include "stdutil.h"
#include "ttyConsole.hpp"
#include "led_blink.hpp"
#include "hardwareConf.hpp"
#include "periodSense.hpp"
#include "rpmMsg.hpp"
#include "userParameters.hpp"
#include "messageImplChibios.hpp"

/*
Connecter sur la carte de dev le chip convertisseur USB série  :
  ftdi RX sur B10 (enlever le jumper)
  ftdi TX sur B11 (enlever le jumper)

  Connecter une LED sur la broche C2 (flash led)
  Connecter PA5 sur PA2, PB14, PC6
  Connecter un cable USB (obligatoire en compilation mode Debug, sinon segfault quand DebugTrace)

*/

/*

  TODO :
 
  * fix mode OPTO : debugger avec l'analyseur logique pour voir si le pb 
    à la décélération est solutionable

  * entrée sensor et sensorless sur deux broches differentes 
    pour chacun des 8 timers

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



  userParam.readConfFromEEprom();
  messageInit();

  if (userParam.getRunningState() == RunningState::Run) {
    rpmStartStreaming();
  } 

  
  
  ledBlink.setFlashes(rpmGetNumTrackedMotors(),
		      userParam.getBaudRate() / 100000);

  /*
    USB_VBUS est connecté à une broche en input pulldown et à BOOT0 à travers un bouton poussoir

    Si  USB_VBUS est à niveau haut à la mise sous tension : 
     si le bouton poussoir est appuyé : le MCU passe en mode bootloader
     sinon, le shell est lancé sur USB

    Sinon (USB_VBUS est à niveau bas à la mise sous tension)
    Si USB_VBUS passe à niveau haut pendant le fonctionnement : on lance un shell
    pour changer les configuration stockées en mémoire flash
   */

  if (palReadLine(LINE_USB_VBUS) == PAL_LOW) {
    palEnableLineEvent(LINE_USB_VBUS, PAL_EVENT_MODE_RISING_EDGE);
    palWaitLineTimeout(LINE_USB_VBUS, TIME_INFINITE);
  }

  userParam.setRunningState(RunningState::Stop);
  consoleInit();
  consoleLaunch();

  palDisableLineEvent(LINE_USB_VBUS);
  palEnableLineEvent(LINE_USB_VBUS, PAL_EVENT_MODE_FALLING_EDGE);
  palWaitLineTimeout(LINE_USB_VBUS, TIME_INFINITE);
  palClearLine(LINE_USB_LED);
  chThdSleepMilliseconds(100);
  systemReset();
}

