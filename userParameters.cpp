#include "userParameters.hpp"

UserParam userParam;
CalculatedParam calcParam;

void UserParam::setMessPerSecond(uint32_t messPerSecond)
{
  ticksBetweenMessages = CH_CFG_ST_FREQUENCY / messPerSecond;
}


void CalculatedParam::cache(void)
{
  const UserParam &up = userParam;
  
  freqAtMaxRpm = (up.getMaxRpm() * up.getMotorNbMagnets()) / 60UL;
  freqAtMinRpm = (up.getMinRpm() * up.getMotorNbMagnets()) / 60UL;
  tickAtMinRpm =  TIMER_FREQ_IN / freqAtMinRpm;

  timDivider = ceilf (tickAtMinRpm / powf(2, TIMER_WIDTH_BITS));
  nbTicksAtMaxRpm =  powf(2.0f, TIMER_WIDTH_BITS) * up.getMinRpm() / up.getMaxRpm();
  widthOneRpm =  TIMER_FREQ_IN  * 60ULL / timDivider / up.getMotorNbMagnets();
}
