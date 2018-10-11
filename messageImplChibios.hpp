#include "serialMsg.hpp"
#include "messageCommonDef.hpp"

#include <ch.h>
#include <hal.h>
#include "globalVar.h"
#include "stdutil.h"


void messageInit(const char* device = nullptr);

// no runOnRecept impl since this message is only meant to be sent
Derive_DynMsg(Errors) 
};

// no runOnRecept impl since this message is only meant to be sent
Derive_DynMsg(Rpms) 
};
