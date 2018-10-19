void messageInit(const char* device)
{
  SystemDependant::initClass(device);

  msgRegister<Msg_Errors>();
  msgRegister<Msg_Rpms>();
  msgRegister<Msg_MessPerSecond>();
  msgRegister<Msg_StartStopMeasure>();
  msgRegister<Msg_MotorParameters>();
  
  FrameMsgReceive::launchMillFrameThread();
}
