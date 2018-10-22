#pragma once
#include "hardwareConf.hpp"


struct __attribute__((packed)) Errors {
  using AType = uint8_t;
  static constexpr size_t ASize = 8;
  uint32_t dynSize=0U;
  
  std::array<AType, ASize> values;
};

struct __attribute__((packed)) Rpms {
  using AType = uint16_t;
  static constexpr size_t ASize = Errors::ASize;
  uint32_t dynSize=0U;
  
  std::array<AType, ASize> values;
};

struct __attribute__((packed)) MessPerSecond {
  uint16_t value;
};

struct __attribute__((packed)) StartStopMeasure {
  RunningState runningState;
};

struct __attribute__((packed)) MotorParameters {
  uint32_t minRpm;
  uint32_t maxRpm;
  uint32_t motorNbMagnets;
  uint32_t nbMotors;
  SensorType sensorType;
};

