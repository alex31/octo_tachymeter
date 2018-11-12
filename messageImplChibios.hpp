#include "serialMsg.hpp"
#include "messageCommonDef.hpp"

#include <ch.h>
#include <hal.h>
#include "globalVar.h"
#include "stdutil.h"
#include "cpp_heap_alloc.hpp"
#include "userParameters.hpp"
#include "rpmMsg.hpp"


void messageInit(const char* device = nullptr);

// no runOnRecept impl since this message is only meant to be sent
Derive_DynMsg(Errors) //{
};

// no runOnRecept impl since this message is only meant to be sent
Derive_DynMsg(Rpms) //{
};

// no runOnRecept impl since this message is only meant to be sent
Derive_Msg(TachoError) //{
};

// no runOnRecept impl since this message is only meant to be sent
Derive_Msg(TachoStates) //{
};

Derive_Msg(MessPerSecond) //{
void  runOnRecept(void) const final {
  auto oldTick = userParam.getTicksBetweenMessages();
  userParam.setMessPerSecond(data->value);
  if (oldTick != userParam.getTicksBetweenMessages()) {
    userParam.storeConfToEEprom();
    DebugTrace("store new conf to eeprom");
  } else {
    DebugTrace("param have not changed");
  }
  DebugTrace("runOnRecept MessPerSecond");
}
};

Derive_Msg(StartStopMeasure) //{
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

Derive_Msg(MotorParameters) //{
void  runOnRecept(void) const final {
  if (userParam.getRunningState() ==  RunningState::Stop) {
    const  UserParam beforeSet = userParam;
    userParam.setMinRpm(data->minRpm);
    userParam.setMaxRpm(data->maxRpm);
    userParam.setMotorNbMagnets(data->motorNbMagnets);
#ifdef TRACE
    userParam.setNbMotors(data->nbMotors % 10);
    userParam.setInterleavedSensor(data->nbMotors > 10);
#else
    userParam.setNbMotors(data->nbMotors);
#endif
    userParam.setSensorType(data->sensorType);
    DebugTrace("runOnRecept MotorParameters");
    DebugTrace("nb motor =%lu interleaved =%d", userParam.getNbMotors(),
	       userParam.getInterleavedSensor());
    if (userParam != beforeSet) {
      DebugTrace("store new conf to eeprom");
      userParam.storeConfToEEprom();
    } else {
      DebugTrace("param have not changed");
    }
  } else {
    FrameMsgSendObject<Msg_TachoError>::send(TachoError("warn: ignoring MotorParameters when running"));
  }
}
};


Derive_Msg(GetTachoStates) //{
void  runOnRecept(void) const final {
  const TachoStates ts = {
    .mp = {
      .minRpm = userParam.getMinRpm(),
      .maxRpm = userParam.getMaxRpm(),
      .motorNbMagnets =  static_cast<uint8_t>(userParam.getMotorNbMagnets()),
      .nbMotors =  static_cast<uint8_t>(userParam.getNbMotors()),
      .sensorType =  userParam.getSensorType()
    },
    .widthOneRpm = calcParam.getWidthOneRpmOpto(),
    .timDivider =  calcParam.getTimDivider(),
    .messPerSecond = CH_CFG_ST_FREQUENCY / userParam.getTicksBetweenMessages(),
    .runningState = userParam.getRunningState()
  };
  DebugTrace("runOnRecept GetTachoStates");
  FrameMsgSendObject<Msg_TachoStates>::send(ts);
}
};

