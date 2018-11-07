#include "userParameters.hpp"
#include <cmath>
#include "messageImplChibios.hpp"

UserParam userParam;
CalculatedParam calcParam;

void UserParam::setMessPerSecond(uint32_t messPerSecond)
{
  if ((messPerSecond != 0) and (messPerSecond <= CH_CFG_ST_FREQUENCY)) {
    ticksBetweenMessages = CH_CFG_ST_FREQUENCY / messPerSecond;
  } else {
    FrameMsgSendObject<Msg_TachoError>::send(TachoError("invalid MessPerSecond value"));
  }
}


bool CalculatedParam::cache(void)
{
  const UserParam &up = userParam;

  if ((up.getMotorNbMagnets() <= 2) or (up.getMotorNbMagnets() > 100)) {
    FrameMsgSendObject<Msg_TachoError>::send(TachoError("invalid MotorNbMagnets value"));
    return false;
  }

  if ((up.getNbMotors() < 1) or (up.getNbMotors() > 8)) {
    FrameMsgSendObject<Msg_TachoError>::send(TachoError("invalid NbMotors value"));
    return false;
  }

  if ((up.getMinRpm() < 10) or (up.getMinRpm() > 10000)) {
    FrameMsgSendObject<Msg_TachoError>::send(TachoError("invalid MinRpm value"));
    return false;
  }

  if ((up.getMaxRpm() < 10) or (up.getMaxRpm() > 1000000)) {
    FrameMsgSendObject<Msg_TachoError>::send(TachoError("invalid MaxRpm value"));
    return false;
  }
  
  freqAtMaxRpm = (up.getMaxRpm() * up.getMotorNbMagnets()) / 60UL;
  freqAtMinRpm = (up.getMinRpm() * up.getMotorNbMagnets()) / 60UL;
  tickAtMinRpm =  TIMER_FREQ_IN / freqAtMinRpm;

  timDivider = ceilf (tickAtMinRpm / powf(2, TIMER_WIDTH_BITS));

  if ((timDivider < 1) or (timDivider > 65535)) {
    FrameMsgSendObject<Msg_TachoError>::send(TachoError("invalid timDivider value"));
    return false;
  }
  
  nbTicksAtMaxRpm =  powf(2.0f, TIMER_WIDTH_BITS) * up.getMinRpm() / up.getMaxRpm();
  if (nbTicksAtMaxRpm > 65535) {
    FrameMsgSendObject<Msg_TachoError>::send(TachoError("invalid nbTicksAtMaxRpm value"));
    return false;
  }
  
  widthOneRpm =  TIMER_FREQ_IN  * 60ULL / timDivider / up.getMotorNbMagnets();

  if (widthOneRpm < 1) {
    FrameMsgSendObject<Msg_TachoError>::send(TachoError("invalid widthOneRpm value"));
    return false;
  }

  return true;
}
