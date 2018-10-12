#pragma once
#include <ch.h>
#include <hal.h>


class UserParam {
public:
  void setMessPerSecond(uint32_t messPerSecond);
  uint16_t getTicksBetweenMessages(void)  {return ticksBetweenMessages;};

private:
  uint32_t ticksBetweenMessages=1000;
};

extern UserParam userParam;

