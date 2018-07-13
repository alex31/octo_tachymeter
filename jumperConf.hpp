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
  constexpr JumperConf(std::array<GpioMask, N> _ms);
  constexpr bool areMasksValid(std::array<GpioMask, N> _ms);
  constexpr bool areMasksValid(void) const  {return invalidMask;};
  ioportmask_t  readConf(size_t index) const;
private:
  const std::array<GpioMask, N> ms;
  const  bool  invalidMask;
};


template<size_t N>
constexpr JumperConf<N>::JumperConf(std::array<GpioMask, N> _ms) : ms(_ms),
								  invalidMask(areMasksValid(_ms))
{
}

template<size_t N>
constexpr bool JumperConf<N>::areMasksValid(std::array<GpioMask, N> _ms)
{
  for (size_t i=0; i<N; i++) {
    const ioportmask_t mask = _ms[i].second;
    const int32_t lsbIndex =  static_cast<int32_t>(__builtin_ffs(mask)) -1;
    if (lsbIndex < 0) 
      return false;
    
    const int32_t msbIndex =  32UL - __builtin_clz(mask);
    const uint32_t width = __builtin_popcount(mask);
    if (static_cast<int32_t>(width) != (msbIndex - lsbIndex))
      return false;

    return true;
  }
}


template<size_t N>
ioportmask_t JumperConf<N>::readConf(size_t index) const
{
  if (index >= N)
    return USHRT_MAX;

  const ioportmask_t mask = ms[index].second;
  const int32_t lsbIndex =  static_cast<int32_t>(__builtin_ffs(mask)) -1;
  const uint32_t width = __builtin_popcount(mask);
  const IOBus busSet = {(stm32_gpio_t *) ms[index].first,
			PAL_GROUP_MASK(width), static_cast<uint32_t>(lsbIndex)};

  return palReadBus(&busSet);
}

