#pragma once
#include "hardwareConf.hpp"

#include <ch.h>
#include <hal.h>

static constexpr uint32_t TIMER_FREQ_IN = STM32_HCLK / 2UL;


class UserParam {
public:
  void setMessPerSecond(uint32_t messPerSecond);
  uint16_t getTicksBetweenMessages(void) const {return ticksBetweenMessages;};

  void setSensorType(const SensorType st) {sensorType=st;};
  SensorType getSensorType(void) const {return sensorType;};

  void setRunningState(const RunningState rs) {runningState=rs;};
  RunningState getRunningState(void) const {return runningState;};

  void setMinRpm(const uint32_t rpm) {minRpm=rpm;};
  uint32_t getMinRpm(void) const {return minRpm;};

  void setMaxRpm(const uint32_t rpm) {maxRpm=rpm;};
  uint32_t getMaxRpm(void) const {return maxRpm;};

  void setMotorNbMagnets(const uint32_t nbm) {motorNbMagnets=nbm;};
  uint32_t getMotorNbMagnets(void) const {return motorNbMagnets;};
  
private:
  uint32_t ticksBetweenMessages = 1000;

  uint32_t minRpm = INIT_MIN_RPM;
  uint32_t maxRpm = INIT_MAX_RPM;
  uint32_t motorNbMagnets = INIT_MOTOR_NB_MAGNETS;
  SensorType sensorType = INIT_SENSOR_TYPE;

  RunningState runningState =  INIT_RUNNING_STATE;
};

extern UserParam userParam;


class CalculatedParam {
public:
  void cache(void);
  uint32_t getFreqAtMaxRpm(void) const {return freqAtMaxRpm;};
  uint32_t getFreqAtMinRpm(void) const {return freqAtMinRpm;};
  uint32_t getTickAtMinRpm(void) const {return tickAtMinRpm;};
  uint32_t getTimDivider(void) const {return timDivider;};
  uint32_t getNbTicksAtMaxRpm(void) const {return nbTicksAtMaxRpm;};
  uint32_t getWidthOneRpm(void) const {return widthOneRpm;};

private:
  uint32_t freqAtMaxRpm;
  uint32_t freqAtMinRpm;
  uint32_t tickAtMinRpm;
  uint32_t timDivider;
  uint32_t nbTicksAtMaxRpm;
  uint32_t widthOneRpm;
};

extern CalculatedParam calcParam;
