#pragma once
#include <type_traits>
#include "frameCommonConf.hpp"

#include "libsrc/serialMsg_conf_lib.hpp"



using SystemDependantTransport = SystemDependant_posixSerial;
using SystemDependantOs = SystemDependant_posix;

class SystemDependant : public SystemDependantTransport, public SystemDependantOs
{
public:
  static void initClass(const char* device) {
    SystemDependantOs::initClass();
    if (SystemDependantTransport::initClass(device, baudRate) == false) {
    SystemDependantOs::abort("serial open failed");
  }
  }
private:
  SystemDependant() = delete;
};

