#pragma once
#include "hardwareConf.hpp"
#include "gpioBus.hpp"

// 3 pins to code number of motor : BUS_NBM[0..3]
// 1 pin to code HALL or ESC mode
static constexpr std::array<GpioMask, 2> JUMPER_BUSES = {{
    {GPIOB_BASE, (1<<BUS_NBM0) | (1<<BUS_NBM1) | (1<<BUS_NBM2)},
    {GPIOB_BASE, (1<<BUS_HALL_OR_ESC)}
  }};


constexpr GpioBus<JUMPER_BUSES.size()> JUMPERS(JUMPER_BUSES);
static_assert(JUMPERS.areMasksValid() == true, "one or more mask are invalid, "
		                             "all zeroes or one not contiguous");

