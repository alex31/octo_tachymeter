#include "serialMsg.hpp"
#include "messageCommonDef.hpp"

#include <ch.h>
#include <hal.h>
#include "globalVar.h"
#include "stdutil.h"
#include "userParameters.hpp"
#include "rpmMsg.hpp"

void messageInit(const char* device = nullptr);

// no runOnRecept impl since this message is only meant to be sent
Derive_DynMsg(Errors) 
};

// no runOnRecept impl since this message is only meant to be sent
Derive_DynMsg(Rpms) 
};

// no runOnRecept impl since this message is only meant to be sent
Derive_Msg(TachoError) 
};

Derive_Msg(MessPerSecond)
void  runOnRecept(void) const final {
  userParam.setMessPerSecond(data->value);
  DebugTrace("runOnRecept MessPerSecond");
}
};

Derive_Msg(StartStopMeasure)
void  runOnRecept(void) const final {
  if (userParam.getRunningState() != data->runningState) {
    userParam.setRunningState(data->runningState);
    if (data->runningState == RunningState::Stop) {
      rpmStopStreaming();
      DebugTrace("runOnRecept Stopping");
    } else if (data->runningState == RunningState::Run) {
      DebugTrace("runOnRecept Starting");
      rpmStartStreaming();
    }
  } else {
    DebugTrace("runOnRecept Unchanged state");
  }
}
};

Derive_Msg(MotorParameters)
void  runOnRecept(void) const final {
  if (userParam.getRunningState() ==  RunningState::Stop) {
    userParam.setMinRpm(data->minRpm);
    userParam.setMaxRpm(data->maxRpm);
    userParam.setMotorNbMagnets(data->motorNbMagnets);
    userParam.setNbMotors(data->nbMotors);
    userParam.setSensorType(data->sensorType);
    DebugTrace("runOnRecept MotorParameters");
  } else {
    FrameMsgSendObject<Msg_TachoError>::send(TachoError("ignoring MotorParameters when running"));
  }
}
};
