#include "userParameters.hpp"
#include <cmath>
#include "messageImplChibios.hpp"
#include "eeprom.h"

UserParam userParam;
CalculatedParam calcParam;

void UserParam::setMessPerSecond(uint32_t messPerSecond)
{
  if ((messPerSecond != 0) and (messPerSecond <= CH_CFG_ST_FREQUENCY)) {
    ticksBetweenMessages = CH_CFG_ST_FREQUENCY / messPerSecond;
  } else {
    FrameMsgSendObject<Msg_TachoError>::send(TachoError("err: invalid MessPerSecond value"));
  }
}


bool UserParam::storeConfToEEprom(void)
{
  const ErrorCond st1 = eepromStore(TACHO_PARAMS, this, sizeof(*this));
  switch (st1) {
  case PROG_OK:
    DebugTrace("DBG> eeprom store ok");
    break;
  case SECTOR_FULL_ERR:
    DebugTrace("DBG> eeprom store full, wipe and store");
    if (eepromWipe() != PROG_OK) {
      FrameMsgSendObject<Msg_TachoError>::send(TachoError("err: eeprom wipe"));
      return false;
    } else {
      if (eepromStore(TACHO_PARAMS, this, sizeof(*this))  != PROG_OK) {
	FrameMsgSendObject<Msg_TachoError>::send(TachoError("err: eeprom store after wipe"));
	return false;
      }
    }
    break;
  default:
    FrameMsgSendObject<Msg_TachoError>::send(TachoError("err: eeprom store"));
    return false;
  };

  return true;
}

bool UserParam::readConfFromEEprom(void)
{
  const ErrorCond st1 = eepromLoad(TACHO_PARAMS, this, sizeof(*this));
  if (st1 > PROG_OK) {
    // first time, should init eeprom
    if (eepromWipe() != PROG_OK) {
      FrameMsgSendObject<Msg_TachoError>::send(TachoError("err: eeprom wipe"));
      return false;
    }
    getWinAvgSize();
    getWinAvgMedianSize();
    if (eepromStore(TACHO_PARAMS, this, sizeof(*this)) != PROG_OK) {
      FrameMsgSendObject<Msg_TachoError>::send(TachoError("err: initial eeprom store"));
    }
  }
  
  PeriodSense::setWinAvgSize(windowFilterSize);
  PeriodSense::setWinAvgMedianSize(medianFilterSize);
  
  return true;
}


bool CalculatedParam::cache(void)
{
  const UserParam &up = userParam;

  if ((up.getMotorNbMagnets() <= 2) or (up.getMotorNbMagnets() > 100)) {
    FrameMsgSendObject<Msg_TachoError>::send(TachoError("err: invalid MotorNbMagnets value"));
    return false;
  }

  if ((up.getNbMotors() < 1) or (up.getNbMotors() > 8)) {
    FrameMsgSendObject<Msg_TachoError>::send(TachoError("err: invalid NbMotors value"));
    return false;
  }

#ifdef TRACE
  if (up.getInterleavedSensor() == true) {
    if ((up.getNbMotors() %2)) {
      FrameMsgSendObject<Msg_TachoError>::send(TachoError("err: invalid odd NbMotors (interleaved)"));
      return false;
    }
  }
#endif
  
  if ((up.getMinRpm() < 10) or (up.getMinRpm() > 10000)) {
    FrameMsgSendObject<Msg_TachoError>::send(TachoError("err: invalid MinRpm value"));
    return false;
  }

  freqAtMaxRpm = (up.getMaxRpm() * up.getMotorNbMagnets()) / 60UL;
  freqAtMinRpm = (up.getMinRpm() * up.getMotorNbMagnets()) / 60UL;
  tickAtMinRpm =  TIMER_FREQ_IN / freqAtMinRpm;

  timDivider = ceilf (tickAtMinRpm / powf(2, TIMER_WIDTH_BITS));

  if ((timDivider < 1) or (timDivider > 65535)) {
    FrameMsgSendObject<Msg_TachoError>::send(TachoError("err: invalid timDivider value"));
    return false;
  }
  
  nbTicksAtMaxRpm =  powf(2.0f, TIMER_WIDTH_BITS) * up.getMinRpm() / up.getMaxRpm();
  if (nbTicksAtMaxRpm >= 65535) {
    FrameMsgSendObject<Msg_TachoError>::send(TachoError("err: invalid nbTicksAtMaxRpm value"));
    return false;
  }

  widthOneRpmHall =  TIMER_FREQ_IN  * 60ULL / timDivider / up.getMotorNbMagnets();
  widthOneRpmOpto =  STM32_HCLK  * 60ULL / up.getMotorNbMagnets();

  const uint32_t bitResolution = logf(widthOneRpmHall/up.getMaxRpm()) / logf(2);
  if (bitResolution < 3) {
    FrameMsgSendObject<Msg_TachoError>::send(TachoError("err: low resolution < 3 bits @maxSpeed"));
    return false;
  } else if (bitResolution < 6) {
    FrameMsgSendObject<Msg_TachoError>::send(TachoError("warn: low resolution < 6 bits @maxSpeed"));
  }

  return true;
}
