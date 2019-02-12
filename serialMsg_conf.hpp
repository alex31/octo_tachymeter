#pragma once

#include "ch.h"
#include "hal.h"
#include <type_traits>

#include "frameCommonConf.hpp"
#include "libsrc/serialMsg_conf_lib.hpp"
#include "userParameters.hpp"

using SystemDependantTransport = SystemDependant_chibiosUART;
using SystemDependantOs = SystemDependant_chibios;

class SystemDependant : public SystemDependantTransport, public SystemDependantOs
{
public:
  static void initClass(const char* device = nullptr) {
    (void) device;
    SystemDependantTransport::initClass(UARTD5, userParam.getBaudRate());
  }
private:
  SystemDependant() = delete;
};


