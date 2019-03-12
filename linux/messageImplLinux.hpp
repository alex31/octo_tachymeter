#include "serialMsg.hpp"
#include "messageCommonDef.hpp"
#include <cstdio>

void messageInit(const char* device);

// id 0
Derive_DynMsg(Errors) 
  void  runOnRecept(void) const final {
  printf ("errors : ");
  for (size_t i=0; i< data->dynSize; i++) {
    printf ("%u : ", data->values[i]);
  }
  printf ("\n");
}
};

// id 1
Derive_DynMsg(Rpms) 
  void  runOnRecept(void) const final {
   printf ("rpms : ");
  for (size_t i=0; i< data->dynSize; i++) {
    printf ("%u : ", data->values[i]);
  }
  printf ("\n");
}
};

// id 2
// no runOnRecept impl since this message is only meant to be sent
Derive_Msg(MessPerSecond)
};

// id 3
// no runOnRecept impl since this message is only meant to be sent
Derive_Msg(StartStopMeasure)
};

// id 4
// no runOnRecept impl since this message is only meant to be sent
Derive_Msg(MotorParameters)
};


// id 5
Derive_Msg(TachoError)
void  runOnRecept(void) const final {
  printf ("Error received : %s \n", data->error);
}
};

// id 6
// no runOnRecept impl since this message is only meant to be sent
Derive_Msg(GetTachoStates)
};

// id 7
// no runOnRecept impl since this message is only meant to be sent
Derive_Msg(TachoStates) 
void  runOnRecept(void) const final {
  printf ("tacho state : \n");
  printf ("running state : %s\n",
	  data->runningState == RunningState::Run ? "Run" : "Stop");
  printf ("widthOneRpm : %d\n", data->widthOneRpm);
  printf ("timDivider : %d\n", data->timDivider);
  printf ("messPerSecond : %d\n", data->messPerSecond);
  printf ("windowSize : %d\n", data->windowSize);
  printf ("medianSize : %d\n", data->medianSize);
  printf ("minRpm : %u\n", data->mp.minRpm);
  printf ("maxRpm : %u\n", data->mp.maxRpm);
  printf ("motorNbMagnets : %u\n", data->mp.motorNbMagnets);
  printf ("nbMotors : %u\n", data->mp.nbMotors);
  printf ("\n");
}
};


// id 8
// no runOnRecept impl since this message is only meant to be sent
Derive_Msg(FilterParam)
};

// id 9
// no runOnRecept impl since this message is only meant to be sent
Derive_Msg(Eeprom)
};


