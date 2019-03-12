#pragma once
#include "periodSense.hpp"
#include "userParameters.hpp"


void		rpmStartStreaming(void);
void		rpmStopStreaming(void);
uint16_t	rpmGetRPM(const size_t index);
#if UPDATE_TOTAL_ERRORS
uint32_t	rpmGetTotalErrors(const size_t index);
#endif
size_t		rpmGetNumTrackedMotors(void);
SensorType	rpmGetSensorType(void);
PeriodSense&    rpmGetPS(const size_t index);
