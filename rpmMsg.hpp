#pragma once
#include "periodSense.hpp"
#include "userParameters.hpp"


void		rpmStartStreaming(void);
void		rpmStopStreaming(void);
uint16_t	rpmGetRPM(size_t index);
size_t		rpmGetNumTrackedMotors(void);
SensorType	rpmGetSensorType(void);
PeriodSense&    rpmGetPS(size_t index);
