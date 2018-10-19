#include "serialMsg.hpp"
#include "messageCommonDef.hpp"

#include <ch.h>
#include <hal.h>
#include "globalVar.h"
#include "stdutil.h"
#include "userParameters.hpp"

void messageInit(const char* device = nullptr);

// no runOnRecept impl since this message is only meant to be sent
Derive_DynMsg(Errors) 
};

// no runOnRecept impl since this message is only meant to be sent
Derive_DynMsg(Rpms) 
};

Derive_Msg(MessPerSecond)
void  runOnRecept(void) const final {
  userParam.setMessPerSecond(data->value);
}
};

Derive_Msg(StartStopMeasure)
void  runOnRecept(void) const final {
  if (userParam.getRunningState() != data->runningState) {
    userParam.setRunningState(data->runningState);
    if (data->runningState == RunningState::Stop) {
      // stop thread
    } else if (data->runningState == RunningState::Run) {
      // launch thread
      // the launch method has to verify that thread is stopped
    }
  }
}
};

Derive_Msg(MotorParameters)
void  runOnRecept(void) const final {
  if (userParam.getRunningState() ==  RunningState::Stop) {
    userParam.setMinRpm(data->minRpm);
    userParam.setMaxRpm(data->maxRpm);
    userParam.setMotorNbMagnets(data->motorNbMagnets);
    userParam.setSensorType(data->sensorType);
  }
}
};
