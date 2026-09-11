#pragma once

#include <array>
#include <cstdint>

namespace bsp {

// Native Win32 XOVERLAPPED storage. Offsets and the seven-DWORD extent follow
// the game's +130/+384/+3C0 blocks; only words[0] is named here (InternalLow).
// Keep the object alive and at the same address until its operation completes.
// Some game reset paths clear only the first five words, not the entire block.
struct XLiveOverlapped {
    std::array<std::uint32_t, 7> words{};
};
static_assert(sizeof(XLiveOverlapped) == 0x1c);

} // namespace bsp
