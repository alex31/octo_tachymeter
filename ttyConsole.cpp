#define _GNU_SOURCE // enable strcasestr
#include <cstdio>
#include <cstdlib>
#include <cctype>
#include "ch.h"
#include "hal.h"
#include "microrl/microrlShell.h"
#include "stdutil.h"
#include "globalVar.h"
#include "printf.h"
#include "ttyConsole.hpp"
#include "etl/cstring.h"
#include <etl/vector.h>
#include "userParameters.hpp"
#include "eeprom.h"

/*===========================================================================*/
/* START OF EDITABLE SECTION                                           */
/*===========================================================================*/

// declaration des prototypes de fonction
// ces declarations sont necessaires pour remplir le tableau commands[] ci-dessous
static void cmd_mem(BaseSequentialStream *lchp, int argc,const char* const argv[]);
#if CH_DBG_THREADS_PROFILING
static void cmd_threads (BaseSequentialStream *lchp, int argc,const char * const argv[]);
#endif
static void cmd_uid(BaseSequentialStream *lchp, int argc,const char* const argv[]);
static void cmd_param(BaseSequentialStream *lchp, int argc,const char* const argv[]);
static void cmd_conf(BaseSequentialStream *lchp, int argc,const char* const argv[]);
static void cmd_restart(BaseSequentialStream *lchp, int argc,const char* const argv[]);

static const ShellCommand commands[] = {
  {"mem", cmd_mem},		// affiche la mémoire libre/occupée
#if  CH_DBG_THREADS_PROFILING
  {"threads", cmd_threads},	// affiche pour chaque thread le taux d'utilisation de la pile et du CPU
#endif
  {"uid", cmd_uid},		// affiche le numéro d'identification unique du MCU
  {"param", cmd_param},		// fonction à but pedagogique qui affiche les
				//   paramètres qui lui sont passés

  {"restart", cmd_restart},	// reboot MCU
  {"conf", cmd_conf},		// gestion de configuration (set et eeprom)
  {NULL, NULL}			// marqueur de fin de tableau
};



/*
  definition de la fonction cmd_param asociée à la commande param (cf. commands[])
  cette fonction a but pédagogique affiche juste les paramètres fournis, et tente
  de convertir les paramètres en entier et en flottant, et affiche le resultat de
  cette conversion. 
  une fois le programme chargé dans la carte, essayer de rentrer 
  param toto 10 10.5 0x10
  dans le terminal d'eclipse pour voir le résultat 
 */
static void cmd_param(BaseSequentialStream *lchp, int argc,const char* const argv[])
{
  if (argc == 0) {  // si aucun paramètre n'a été passé à la commande param 
    chprintf(lchp, "pas de paramètre en entrée\r\n");
  } else { // sinon (un ou plusieurs pararamètres passés à la commande param 
    for (int argn=0; argn<argc; argn++) { // pour tous les paramètres
      chprintf(lchp, "le parametre %d/%d est %s\r\n", argn, argc-1, argv[argn]); // afficher

      // tentative pour voir si la chaine peut être convertie en nombre entier et en nombre flottant
      int entier = atoi (argv[argn]); // atoi converti si c'est possible une chaine en entier
      float flottant = atof (argv[argn]); // atof converti si c'est possible une chaine en flottant

      chprintf(lchp, "atoi(%s) = %d ;; atof(%s) = %.3f\r\n",
		argv[argn], entier, argv[argn], flottant);
    }
  }
}

/*
  conf :

  commandes sans arguments
  show
  store
  load
  wipe
  erase

  pour les commandes suivantes : cmd val : affecte la valeur, cmd : affiche la valeur
  magnet 
  motor
  window
  median
  rate
  baud
  smin
  smax
 */

using pGetFunc_t = uint32_t (*) (void);
using pSetFunc_t  = void (*) (uint32_t);
using commandStr_t = etl::string<6>;

struct ConfCommand {
  const char* name;
  const pGetFunc_t  get_f=nullptr;
  const pSetFunc_t   set_f=nullptr;
  const int valMin=0;
  const int valMax=0;
};

constexpr std::array<ConfCommand, 14> map2 = {{
    {
      .name = "magnet",
      .get_f = [] {return userParam.getMotorNbMagnets();},
      .set_f =  [] (uint32_t v) {userParam.setMotorNbMagnets(v);},
      2, 60
    } ,
    {
      .name = "motor", 
      .get_f = [] {return userParam.getNbMotors();},
      .set_f =  [] (uint32_t v) {userParam.setNbMotors(v);},
      1, 8
    },
    {
      .name = "window", 
      .get_f = [] {return userParam.getWinAvgSize();},
      .set_f =  [] (uint32_t v) {userParam.setWinAvgSize(v);},
      1, 16
    },
    {
      .name = "median", 
      .get_f = [] {return userParam.getWinAvgMedianSize();},
      .set_f =  [] (uint32_t v) {userParam.setWinAvgMedianSize(v);},
      0, 4
    },
    {
      .name = "rate", 
      .get_f = [] {return CH_CFG_ST_FREQUENCY / userParam.getTicksBetweenMessages();},
      .set_f =  [] (uint32_t v) {userParam.setMessPerSecond(v);},
      1, 10'000
    },
    {
      .name = "baud", 
      .get_f = [] {return userParam.getBaudRate();},
      .set_f =  [] (uint32_t v) {userParam.setBaudRate(v);},
      9600, 1000'000
    },
    {
      .name = "smin", 
      .get_f = [] {return userParam.getMinRpm();},
      .set_f =  [] (uint32_t v) {userParam.setMinRpm(v);},
      100,10'000
    },
    {
      .name = "smax", 
      .get_f = [] {return userParam.getMaxRpm();},
      .set_f =  [] (uint32_t v) {userParam.setMaxRpm(v);},
      100, 100'000
    },
    {
      .name = "show", 
      .set_f =   [] ([[maybe_unused]]  uint32_t v) {
	for (const auto & e : map2) {
	  if (e.get_f != nullptr)
	    chprintf (chp, "%s = %lu [%d .. %d]\r\n", e.name, e.get_f(),
		    e.valMin, e.valMax);
	}
      }
    },
    {
      .name = "store", 
      .set_f =  [] ([[maybe_unused]] uint32_t v) {
	chprintf(chp, "eeprom store : %s\r\n",
		 userParam.storeConfToEEprom() ? "SUCCESS" : "ERROR");
      }
    },
    {
      .name = "load", 
      .set_f =  [] ([[maybe_unused]] uint32_t v) {
	chprintf(chp, "eeprom load : %s\r\n",
	userParam.readConfFromEEprom() ? "SUCCESS" : "ERROR");
      }
    },
    {
      .name = "wipe", 
      .set_f =  [] ([[maybe_unused]] uint32_t v) {
	chprintf (chp, "eeprom wipe : %s",
		  eepromWipe() != PROG_OK ? "ERROR" : "SUCCESS");	  
      }
    },
    {
      .name = "erase", 
      .set_f =  [] ([[maybe_unused]] uint32_t v) {
	chprintf (chp, "eeprom erase : %s",
		  eepromErase() != PROG_OK ? "ERROR" : "SUCCESS");	  
      }
    },
    {
      .name = "usage", 
      .set_f =  [] ([[maybe_unused]] uint32_t v) {
	chprintf(chp, 
	       "change or show parameters, load/save from eeprom, command are\r\n"
	       "\r\n"
	       "magnet : number of magnets on the motor\r\n"
	       "motor  : number of motors on the UAV\r\n"
	       "\r\n"
	       "smin   : minimum rpm at which we want to have valid speed value\r\n"
	       "smax   : maximum rpm at which we want to have valid speed value\r\n"
	       "\r\n"
	       "rate   : number of messages by second sent on the uart\r\n"
	       "baud   : baud rate for the serial line\r\n"
	       "\r\n"
	       "window : number of samples in the window of the low pass filter\r\n"
	       "median : number of eliminated samples at each side of the median filter\r\n"
	       "       : number of averaged samples will be window-(2*median)\r\n"
	       "       : window=1, median=0 will switch off low pass filter\r\n"
	       "\r\n"
	       "\r\n"
	       "show   : show all current parameters\r\n"
	       "store  : store current parameters in eeprom\r\n"
	       "load   : load parameters from eeprom\r\n"
	       "wipe   : compress data in eeprom (no data loss)\r\n"
	       "erase  : completely erase all data in eeprom\r\n\r\n"
	       );
      }
    }
  }};

static void cmd_conf([[maybe_unused]] BaseSequentialStream *lchp, int argc,const char* const argv[])
{
 const commandStr_t cmd = (argc < 1) ? "usage" : argv[0];
 etl::vector<const ConfCommand*, 16> matches;

 // DebugTrace("DBG> size matches= %u map2= %u\r\n", sizeof(matches), sizeof(map2));

 // look for command in table entry. Command can be abbreviated if there is no ambibuity
 // this will feed a vector with all matching commands
 etl::transform_if(map2.begin(), map2.end(), std::back_inserter(matches),
		   [](const auto &i) {return &i;},
		   [&cmd](const auto &i) {return strcasestr(i.name, cmd.c_str()) == i.name;});

      
 switch(matches.size()) {
 case 0: 
   chprintf(chp, "command %s not found\r\n", cmd.c_str());
   break;

  case 1: {
    const auto [name, get_f, set_f, min, max] = *matches[0];
    const bool noArgActionOnly = (get_f == nullptr);
    if (argc >= 2) {
      if (noArgActionOnly) {
	chprintf(chp, "command %s cannot take argument\r\n", cmd.c_str());
      } else {
	const int val = atoi(argv[1]);
	if (val < min) {
	  chprintf(chp, "command %s minimum value is %d\r\n", cmd.c_str(), min);
	} else if  (val > max) {
	  chprintf(chp, "command %s maximum value is %d\r\n", cmd.c_str(), max);
	} else {
	  set_f(val);
	}
      }
    } else if (noArgActionOnly) {
      set_f(0);
    } else {
      chprintf(chp, "%s = %lu\r\n", cmd.c_str(), get_f());
    }
  }
    break;
    
  default:
    chprintf(chp, "command %s is ambiguous : %u matches : ", cmd.c_str(), matches.size());
    for (const auto &m : matches) 
      chprintf(chp, "%s, ", m->name);
    chprintf(chp, "\r\n");
  } 
  
}

static void cmd_restart(BaseSequentialStream *lchp, int argc,const char* const argv[])
{
  (void) lchp;
  (void) argc;
  (void) argv;
  systemReset();
}



/*
  
 */


/*===========================================================================*/
/* START OF PRIVATE SECTION  : DO NOT CHANGE ANYTHING BELOW THIS LINE        */
/*===========================================================================*/

/*===========================================================================*/
/* Command line related.                                                     */
/*===========================================================================*/


#define SHELL_WA_SIZE   THD_WORKING_AREA_SIZE(4096)




#ifndef CONSOLE_DEV_USB
#define  CONSOLE_DEV_USB 0
#endif

#if CONSOLE_DEV_USB == 0
static const SerialConfig ftdiConfig =  {
  115200,
  0,
  USART_CR2_STOP1_BITS | USART_CR2_LINEN,
  0
};
#endif


#define MAX_CPU_INFO_ENTRIES 20

typedef struct _ThreadCpuInfo {
  float    ticks[MAX_CPU_INFO_ENTRIES];
  float    cpu[MAX_CPU_INFO_ENTRIES];
  float    totalTicks;
  _ThreadCpuInfo () {
    for (auto i=0; i< MAX_CPU_INFO_ENTRIES; i++) {
      ticks[i] = 0.0f;
      cpu[i] = -1.0f;
    }
    totalTicks = 0.0f;
  }
} ThreadCpuInfo ;
  
#if CH_DBG_THREADS_PROFILING
static void stampThreadCpuInfo (ThreadCpuInfo *ti);
static float stampThreadGetCpuPercent (const ThreadCpuInfo *ti, const uint32_t idx);
#endif

static void cmd_uid(BaseSequentialStream *lchp, int argc,const char* const argv[]) {
  (void)argv;
  if (argc > 0) {
     chprintf(lchp, "Usage: uid\r\n");
    return;
  }

  for (uint32_t i=0; i< UniqProcessorIdLen; i++)
    chprintf(lchp, "[%x] ", UniqProcessorId[i]);
  chprintf(lchp, "\r\n");
}



static void cmd_mem(BaseSequentialStream *lchp, int argc,const char* const argv[]) {
  (void)argv;
  if (argc > 0) {
    chprintf(lchp, "Usage: mem\r\n");
    return;
  }

  chprintf(lchp, "core free memory : %u bytes\r\n", chCoreStatus());
  chprintf(lchp, "heap free memory : %u bytes\r\n", getHeapFree());

  void * ptr1 = malloc_m (100);
  void * ptr2 = malloc_m (100);

  chprintf(lchp, "(2x) malloc_m(1000) = %p ;; %p\r\n", ptr1, ptr2);
  chprintf(lchp, "heap free memory : %d bytes\r\n", getHeapFree());

  free_m (ptr1);
  free_m (ptr2);
}



#if  CH_DBG_THREADS_PROFILING
static void cmd_threads(BaseSequentialStream *lchp, int argc,const char* const argv[]) {
  static const char *states[] = {THD_STATE_NAMES};
  Thread *tp = chRegFirstThread();
  (void)argv;
  (void)argc;
  float totalTicks=0;
  float idleTicks=0;

  static ThreadCpuInfo threadCpuInfo ;
  
  stampThreadCpuInfo (&threadCpuInfo);
  
  chprintf(lchp, "    addr    stack  frestk prio refs  state        time \t percent        name\r\n");
  uint32_t idx=0;
  do {
    chprintf(lchp, "%.8lx %.8lx %6lu %4lu %4lu %9s %9lu   %.1f    \t%s\r\n",
	      (uint32_t)tp, (uint32_t)tp->ctx.sp,
	      get_stack_free (tp),
	      (uint32_t)tp->prio, (uint32_t)(tp->refs - 1),
	      states[tp->state], (uint32_t)tp->time, 
	      stampThreadGetCpuPercent (&threadCpuInfo, idx),
	      chRegGetThreadName(tp));
    totalTicks+= (float) tp->time;
    if (strcmp (chRegGetThreadName(tp), "idle") == 0)
      idleTicks =  (float) tp->time;
    tp = chRegNextThread ((Thread *)tp);
    idx++;
  } while (tp != NULL);

  const float idlePercent = (idleTicks*100.f)/totalTicks;
  const float cpuPercent = 100.f - idlePercent;
  chprintf(lchp, "\r\ncpu load = %.2f%%\r\n", cpuPercent);
}
#endif

static const ShellConfig shell_cfg1 = {
#if CONSOLE_DEV_USB == 0
  (BaseSequentialStream *) &CONSOLE_DEV_SD,
#else
  (BaseSequentialStream *) &SDU1,
#endif
  commands
};



void consoleInit (void)
{
  /*
   * Activates the USB driver and then the USB bus pull-up on D+.
   * USBD1 : FS, USBD2 : HS
   */

#if CONSOLE_DEV_USB != 0
  usbSerialInit(&SDU1, &USBDRIVER); 
  chp = (BaseSequentialStream *) &SDU1;
#else
  sdStart(&CONSOLE_DEV_SD, &ftdiConfig);
  chp = (BaseSequentialStream *) &CONSOLE_DEV_SD;
#endif
  /*
   * Shell manager initialization.
   */
  shellInit();
}


void consoleLaunch (void)
{
  Thread *shelltp = NULL;

 
#if CONSOLE_DEV_USB != 0
   while (TRUE) {
    if (!shelltp) {
      systime_t time=90;


      while (usbGetDriver()->state != USB_ACTIVE) {
	if (time != 100) {
	  time++;
	  chThdSleepMilliseconds(100);
	} else {
	  time=90;
	  //usbSerialReset(&SDU1);
	}
      }
      
      // activate driver, giovani workaround
      chnGetTimeout(&SDU1, TIME_IMMEDIATE);
      while (!isUsbConnected()) {
	chThdSleepMilliseconds(100);
      }
      
      shelltp = shellCreate(&shell_cfg1, SHELL_WA_SIZE, NORMALPRIO);
    } else if (shelltp && (chThdTerminated(shelltp))) {
      chThdRelease(shelltp);    /* Recovers memory of the previous shell.   */
      shelltp = NULL;           /* Triggers spawning of a new shell.        */
    }
    chThdSleepMilliseconds(100);
  }

#else

   if (!shelltp) {
     shelltp = shellCreate(&shell_cfg1, SHELL_WA_SIZE, NORMALPRIO);
   } else if (chThdTerminated(shelltp)) {
     chThdRelease(shelltp);    /* Recovers memory of the previous shell.   */
     shelltp = NULL;           /* Triggers spawning of a new shell.        */
   }
   chThdSleepMilliseconds(100);
   
#endif //CONSOLE_DEV_USB

}

#if  CH_DBG_THREADS_PROFILING
static void stampThreadCpuInfo (ThreadCpuInfo *ti)
{
  const Thread *tp =  chRegFirstThread();
  uint32_t idx=0;
  
  float totalTicks =0;
  do {
    totalTicks+= (float) tp->time;
    ti->cpu[idx] = (float) tp->time - ti->ticks[idx];;
    ti->ticks[idx] = (float) tp->time;
    tp = chRegNextThread ((Thread *)tp);
    idx++;
  } while ((tp != NULL) && (idx < MAX_CPU_INFO_ENTRIES));
  
  const float diffTotal = totalTicks- ti->totalTicks;
  ti->totalTicks = totalTicks;
  
  tp =  chRegFirstThread();
  idx=0;
  do {
    ti->cpu[idx] =  (ti->cpu[idx]*100.f)/diffTotal;
    tp = chRegNextThread ((Thread *)tp);
    idx++;
  } while ((tp != NULL) && (idx < MAX_CPU_INFO_ENTRIES));
}


static float stampThreadGetCpuPercent (const ThreadCpuInfo *ti, const uint32_t idx)
{
  if (idx >= MAX_CPU_INFO_ENTRIES) 
    return -1.f;

  return ti->cpu[idx];
}
#endif
