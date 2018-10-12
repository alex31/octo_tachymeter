void messageInit(const char* device)
{
  SystemDependant::initClass(device);

  msgRegister<Msg_Errors>();
  msgRegister<Msg_Rpms>();
  msgRegister<Msg_MessPerSecond>();
  FrameMsgReceive::launchMillFrameThread();
}
