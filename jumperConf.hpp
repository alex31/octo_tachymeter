#pragma once
#include "hardwareConf.hpp"
#include "gpioBus.hpp"

constexpr GpioBus<JUMPER_BUSES.size()> JUMPERS(JUMPER_BUSES);
static_assert(JUMPERS.areMasksValid() == true, "one or more mask are invalid, "
		                             "all zeroes or one not contiguous");

