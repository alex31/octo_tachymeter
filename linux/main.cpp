#include "serialMsg.hpp"
#include <iostream>
#include <cstdlib>
#include <stdint.h>
#include "messageImplLinux.hpp"





int main(int argc, char* argv[])
{
  const char* device = (argc == 1) ? "/dev/ttyUSB0" : argv[1];
  std::cout << "device = " << device << std::endl;
  
  messageInit(device);

  while (true) {
    FrameMsgSendObject<Msg_MessPerSecond>::send({.value = 10});
    sleep (5);
    FrameMsgSendObject<Msg_MessPerSecond>::send({.value = 1000});
    sleep (5);
  }
}
