#pragma once
#include "hardwareConf.hpp"

#ifdef __gnu_linux__
#include <bsd/string.h>
#endif

/*
  after having defined message content here you need to :
  ° define behavior in messageImplChibios.hpp, linux/messageImplLinux.hpp
  ° register in common/messageCommonRegister.cpp
 */

// id 0
struct __attribute__((packed)) Errors {
  using AType = uint8_t;
  static constexpr size_t ASize = 8;
  uint8_t  dynSize=0U;
  
  std::array<AType, ASize> values;
};

// id 1
struct __attribute__((packed)) Rpms {
  using AType = uint16_t;
  static constexpr size_t ASize = Errors::ASize;
  uint8_t dynSize=0U;
  
  std::array<AType, ASize> values;
};

// id 2
struct __attribute__((packed)) MessPerSecond {
  uint16_t value;
};

// id 3
struct __attribute__((packed)) StartStopMeasure {
  RunningState runningState;
};

// id 4
struct __attribute__((packed)) MotorParameters {
  uint32_t minRpm;
  uint32_t maxRpm;
  uint8_t motorNbMagnets;
  uint8_t nbMotors;
  SensorType sensorType;
};

// id 5
struct __attribute__((packed)) TachoError {
  TachoError(void) {error[0]=0;};
  TachoError(const char* er) {strlcpy(error, er, sizeof(error));}
  char error[48];
};

// id 6
struct __attribute__((packed)) GetTachoStates {
  uint8_t dummy=0;
};

// id 7
struct __attribute__((packed)) TachoStates {
  MotorParameters mp;
  uint32_t widthOneRpm;
  uint32_t timDivider;
  uint32_t messPerSecond;
  uint8_t windowSize;
  uint8_t medianSize;
  RunningState runningState;
};

// id 8
struct __attribute__((packed)) FilterParam {
  uint8_t windowSize;
  uint8_t medianSize;
};
