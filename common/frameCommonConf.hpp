#include <stdint.h>
#include <array>

constexpr uint16_t maxPayloadLen = 20;
constexpr uint16_t nbMaxMessageIds = 16;
constexpr uint32_t baudRate = 230400;
constexpr std::array<uint8_t, 2> startSyncValue = {0xFE, 0xED};
