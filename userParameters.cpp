#include "userParameters.hpp"

UserParam userParam;

void UserParam::setMessPerSecond(uint32_t messPerSecond)
{
  ticksBetweenMessages = CH_CFG_ST_FREQUENCY / messPerSecond;
}
