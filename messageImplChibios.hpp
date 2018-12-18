#include "serialMsg.hpp"
#include "messageCommonDef.hpp"

#include <ch.h>
#include <hal.h>
#include "globalVar.h"
#include "stdutil.h"
#include "cpp_heap_alloc.hpp"
#include "userParameters.hpp"
#include "rpmMsg.hpp"
#include "eeprom.h"


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
  userParam.setMessPerSecond(data->value);
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
    .windowSize = static_cast<uint8_t>(userParam.getWinAvgSize()),
    .medianSize = static_cast<uint8_t>(userParam.getWinAvgMedianSize()),
    .runningState = userParam.getRunningState()
  };
  DebugTrace("runOnRecept GetTachoStates");
  FrameMsgSendObject<Msg_TachoStates>::send(ts);
}
};

Derive_Msg(FilterParam) //{
void  runOnRecept(void) const final {
  userParam.setWinAvgSize(data->windowSize);
  userParam.setWinAvgMedianSize(data->medianSize);
  DebugTrace("runOnRecept FilterParam w=%d m=%d", data->windowSize, data->medianSize);
}
};

Derive_Msg(Eeprom) //{
void  runOnRecept(void) const final {
  DebugTrace("runOnRecept Eeprom Command c=%d", static_cast<int>(data->command));
  switch (data->command) {
  case EepromCommand::Store :
    userParam.storeConfToEEprom();
    break;
  case EepromCommand::Load :
    userParam.readConfFromEEprom();
    break;
  case EepromCommand::Wipe :
    if (eepromWipe() != PROG_OK) {
      FrameMsgSendObject<Msg_TachoError>::send(TachoError("err: eeprom wipe"));
    }
    break;
  case EepromCommand::Erase :
    if (eepromErase() != PROG_OK) {
      FrameMsgSendObject<Msg_TachoError>::send(TachoError("err: eeprom erase"));
    }
    break;
 default:
   FrameMsgSendObject<Msg_TachoError>::send(TachoError("err: eeprom unknown command"));
  }
}
};
