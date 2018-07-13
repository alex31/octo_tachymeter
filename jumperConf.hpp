#pragma once
#include "hardwareConf.hpp"
#include <ch.h>
#include <hal.h>
#include <utility>  
#include <array>  
#include <climits>


using GpioMask = std::pair<ioportid_t, ioportmask_t>;

template<size_t N>
class JumperConf
{
public:
  JumperConf(std::array<GpioMask, N> ms);
  ioportmask_t  readConf(size_t index);
private:
  std::array<IOBus, N> busSet;
};


template<size_t N>
JumperConf<N>::JumperConf(std::array<GpioMask, N> ms)
{
  for (size_t i=0; i<N; i++) {
    const ioportid_t port = ms[i].first;
    const ioportmask_t mask = ms[i].second;
    const int32_t lsbIndex =  static_cast<int32_t>(__builtin_ffs(mask)) -1;
    if (lsbIndex < 0)
      chSysHalt("JumperConf null mask not allowed");
    
    const int32_t msbIndex =  32UL - __builtin_clz(mask);
    const uint32_t width = __builtin_popcount(mask);
    if (static_cast<int32_t>(width) != (msbIndex - lsbIndex))
      chSysHalt("JumperConf : all bits in a bus should be contigous");

    busSet[i] = {port, PAL_GROUP_MASK(width), static_cast<uint32_t>(lsbIndex)};
  }
}


template<size_t N>
ioportmask_t JumperConf<N>::readConf(size_t index)
{
  if (index >= N)
    return USHRT_MAX;
  return palReadBus(&busSet[index]);
}
