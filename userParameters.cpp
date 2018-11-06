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


void CalculatedParam::cache(void)
{
  const UserParam &up = userParam;

  if ((up.getMotorNbMagnets() <= 2) or (up.getMotorNbMagnets() > 100)) {
    FrameMsgSendObject<Msg_TachoError>::send(TachoError("invalid MotorNbMagnets value"));
    return;
  }

  if ((up.getNbMotors() < 1) or (up.getNbMotors() > 8)) {
    FrameMsgSendObject<Msg_TachoError>::send(TachoError("invalid NbMotors value"));
    return;
  }

  if ((up.getMinRpm() < 10) or (up.getMinRpm() > 10000)) {
    FrameMsgSendObject<Msg_TachoError>::send(TachoError("invalid MinRpm value"));
    return;
  }

  if ((up.getMaxRpm() < 10) or (up.getMaxRpm() > 100000)) {
    FrameMsgSendObject<Msg_TachoError>::send(TachoError("invalid MaxRpm value"));
    return;
  }
  
  freqAtMaxRpm = (up.getMaxRpm() * up.getMotorNbMagnets()) / 60UL;
  freqAtMinRpm = (up.getMinRpm() * up.getMotorNbMagnets()) / 60UL;
  tickAtMinRpm =  TIMER_FREQ_IN / freqAtMinRpm;

  timDivider = ceilf (tickAtMinRpm / powf(2, TIMER_WIDTH_BITS));
  nbTicksAtMaxRpm =  powf(2.0f, TIMER_WIDTH_BITS) * up.getMinRpm() / up.getMaxRpm();
  widthOneRpm =  TIMER_FREQ_IN  * 60ULL / timDivider / up.getMotorNbMagnets();
}
