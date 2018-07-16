#pragma once
#include "periodSense.hpp"

enum class SensorType {Hall_effect, Esc_coupler, No_Init};

void		rpmStartStreaming(void);
uint16_t	rpmGetRPM(size_t index);
size_t		rpmGetNumTrackedMotors(void);
SensorType	rpmGetSensorType(void);
PeriodSense&    rpmGetPS(size_t index);
