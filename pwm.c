#include <ch.h>
#include <hal.h>
#include <stdnoreturn.h>
#include <math.h>
#include "globalVar.h"
#include "stdutil.h"
#include "potentiometre.h"
#include "pwm.h"


static uint32_t newFreq=0;


// pour les parties 1 (persistance) et 2 (servo) et 3 (il variation continue du pwm)
// n'y a pas besoin de modifier  la structure de configuration.
// Il n'y a que dans la partie Utilisation des 4 canaux
// que le champ .channels de cette structure devra être modifié
static PWMConfig pwmcfg = {	// pwm d'une frequence d'un hz et 10000 pas de quantification
  .frequency = 2000*10,        
  .period    = 10       , 
  .callback  = NULL,	     //   pas de callback de passage à l'etat actif
  .channels  = {
    // sortie active, polarité normale, pas de callback
    {.mode = PWM_OUTPUT_ACTIVE_HIGH, .callback = NULL},
    // sortie inactive
    {.mode = PWM_OUTPUT_DISABLED, .callback = NULL},
    // sortie inactive
    {.mode = PWM_OUTPUT_DISABLED, .callback = NULL},
    // sortie inactive
    {.mode = PWM_OUTPUT_DISABLED, .callback = NULL}
  },
  .cr2  = 0, // doit être initialisé à 0 (voir stm32f4 reference manuel)
  .dier = 0  // doit être initialisé à 0 (voir stm32f4 reference manuel)
};

static THD_WORKING_AREA(waPwmCommand, 512);
static noreturn void pwmCommand(void *arg);

void launchPwm ()
{
  initPotentiometre();
  pwmStart(&PWMD2, &pwmcfg);
  chThdCreateStatic(waPwmCommand, sizeof(waPwmCommand), NORMALPRIO, pwmCommand, NULL);
}



uint32_t pwmGetFreq(void)
{
  return newFreq;
}


static noreturn void pwmCommand(void *arg) 
{

  (void)arg;
  chRegSetThreadName("pwmCommand");

  while (true) {
    // renvoie un float entre 0.0f quand le potentiometre est tourné à gauche
    //                     et 1.0f quand il est tourné à droite
    const float potPitch =  getPotValue(); 

    newFreq = (potPitch * 2000) + 30; // entre 30hz et 3030hz
    pwmChangeFrequency(&PWMD2, newFreq);
    pwmEnableChannel(&PWMD2, 0, PWM_PERCENTAGE_TO_WIDTH(&PWMD2, 5000));
 
    
    // la frequence  maximum pour changer la valeur du pwm est la valeur du pwm :
    // si le pwm est à 50hz, on ne doit pas le changer plus de 50 fois par seconde
    chThdSleepMilliseconds (10); 
  }
  
}
 
