#pragma once
#include "hardwareConf.hpp"

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

