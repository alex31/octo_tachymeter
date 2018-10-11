#pragma once


struct __attribute__((packed)) Errors {
  using AType = uint8_t;
  static constexpr size_t ASize = 8;
  uint32_t dynSize=0U;
  
  std::array<AType, ASize> values;
};

struct __attribute__((packed)) Rpms {
  using AType = uint16_t;
  static constexpr size_t ASize = Errors::ASize;
  uint32_t dynSize=0U;
  
  std::array<AType, ASize> values;
};
