#include "serialMsg.hpp"
#include "messageCommonDef.hpp"
#include <cstdio>

void messageInit(const char* device);

Derive_DynMsg(Errors) 
  void  runOnRecept(void) const final {
  printf ("errors : ");
  for (size_t i=0; i< data->dynSize; i++) {
    printf ("%u : ", data->values[i]);
  }
  printf ("\n");
}
};

Derive_DynMsg(Rpms) 
  void  runOnRecept(void) const final {
   printf ("rpms : ");
  for (size_t i=0; i< data->dynSize; i++) {
    printf ("%u : ", data->values[i]);
  }
  printf ("\n");
}
};
