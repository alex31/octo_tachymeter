#pragma once
#include "hardwareConf.hpp"
#include <ch.h>
#include <hal.h>
#include <utility>  
#include <array>  
#include <climits>


using GpioMask = std::pair<uint32_t, ioportmask_t>;

template<size_t N>
class JumperConf
{
public:
  constexpr JumperConf(const std::array<GpioMask, N>& ms);
  constexpr bool areMasksValid(const std::array<GpioMask, N>& ms);
  constexpr bool areMasksValid(void) const  {return invalidMask;};
  ioportmask_t  readConf(size_t index) const;
private:
  uint32_t     baseOffsets[N];
  uint32_t     lsbIndexs[N];
  uint32_t     widths[N];
  const  bool  invalidMask;
};


template<size_t N>
constexpr JumperConf<N>::JumperConf(const std::array<GpioMask, N>& ms) :  baseOffsets{0},
								   lsbIndexs{0},
								   widths{0},
								   invalidMask(areMasksValid(ms))
{
}

template<size_t N>
constexpr bool JumperConf<N>::areMasksValid(const std::array<GpioMask, N>& ms)
{
  for (size_t i=0; i<N; i++) {
    const ioportmask_t mask = ms[i].second;
    const int32_t lsbIndex =  static_cast<int32_t>(__builtin_ffs(mask)) -1;
    const int32_t msbIndex =  32UL - __builtin_clz(mask);
    const uint32_t width = __builtin_popcount(mask);
    baseOffsets[i] = ms[i].first;
    lsbIndexs[i] =  static_cast<uint32_t>(lsbIndex);
    widths[i] = width;
    
    if (lsbIndex < 0) 
      return false;
    
    if (static_cast<int32_t>(width) != (msbIndex - lsbIndex))
      return false;
  }
  return true;
}


template<size_t N>
ioportmask_t JumperConf<N>::readConf(size_t index) const
{
  if (index >= N)
    return USHRT_MAX;

  const uint32_t lsbIndex =  lsbIndexs[index];
  const uint32_t width = widths[index];
  const uint32_t baseOffset = baseOffsets[index];

  const IOBus busSet = {(stm32_gpio_t *) baseOffset, PAL_GROUP_MASK(width), lsbIndex};

  return palReadBus(&busSet);
}

