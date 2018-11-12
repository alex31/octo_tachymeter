#pragma once
#include "hardwareConf.hpp"

#include <ch.h>
#include <hal.h>
#include <cstring>


class UserParam {
public:
  bool operator ==(const UserParam& up) {return (memcmp(this, &up, sizeof(up)) == 0);};
  bool operator !=(const UserParam& up) {return not (*this == up);};
  
  void setMessPerSecond(uint32_t messPerSecond);
  uint32_t getTicksBetweenMessages(void) const {return ticksBetweenMessages;};

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

  void setNbMotors(const uint32_t nb) {nbMotors=nb;};
  uint32_t getNbMotors(void) const {return nbMotors;};

#ifdef TRACE
  void setInterleavedSensor(const bool is) {interleavedSensor=is;};
  bool getInterleavedSensor(void) const {return interleavedSensor;};
#endif
  
  bool readConfFromEEprom(void);
  bool storeConfToEEprom(void);
  
  
private:
  uint32_t ticksBetweenMessages = CH_CFG_ST_FREQUENCY / INIT_MESS_PER_SECOND;

  uint32_t minRpm = INIT_MIN_RPM;
  uint32_t maxRpm = INIT_MAX_RPM;
  uint32_t motorNbMagnets = INIT_MOTOR_NB_MAGNETS;
  uint32_t nbMotors = INIT_MOTOR_NB_MOTORS;
  SensorType sensorType = INIT_SENSOR_TYPE;

  RunningState runningState =  INIT_RUNNING_STATE;
#ifdef TRACE
  bool	       interleavedSensor = false;
#endif
};

extern UserParam userParam;


class CalculatedParam {
public:
  bool	   cache(void);
  uint32_t getFreqAtMaxRpm(void) const {return freqAtMaxRpm;};
  uint32_t getFreqAtMinRpm(void) const {return freqAtMinRpm;};
  uint32_t getTickAtMinRpm(void) const {return tickAtMinRpm;};
  uint32_t getTimDivider(void) const {return timDivider;};
  uint32_t getNbTicksAtMaxRpm(void) const {return nbTicksAtMaxRpm;};
  uint32_t getWidthOneRpmHall(void) const {return widthOneRpmHall;};
  uint32_t getWidthOneRpmOpto(void) const {return widthOneRpmOpto;};

private:
  uint32_t freqAtMaxRpm;
  uint32_t freqAtMinRpm;
  uint32_t tickAtMinRpm;
  uint32_t timDivider;
  uint32_t nbTicksAtMaxRpm;
  uint32_t widthOneRpmHall;
  uint32_t widthOneRpmOpto;
};

extern CalculatedParam calcParam;
