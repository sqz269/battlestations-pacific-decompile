#pragma once

#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error These native ABI leaves require MSVC Win32.
#endif

// Complete 00B3FFA0..00B3FFA3. ECX actual backing storage, m32real at +0x18
// pushed to x87 ST0, plain RET. Float is the Source interface hypothesis;
// no original class or semantic C++ prototype is recovered.
extern "C" float __fastcall bsp_tagged_abi_b3_00b3ffa0(const void* backing);

// Complete 00B3D5E0..00B3D634. This describes the PHYSICAL four-word stack
// envelope, not a recovered four-argument C++ prototype or an unused receiver.
// Entry ESP+4 is actual destination; ESP+0x10 is actual source. The middle
// words are not directly read; both remain explicit, caller-supplied cells
// because RET 0x10 consumes the whole envelope. No defaults or adapter.
// The 0x10-byte local scratch and interleaved accesses are observable under
// arbitrary backed aliases. EAX returns the captured destination address.
extern "C" void* __stdcall bsp_tagged_abi_b3_00b3d5e0(
    void* destination, std::uint32_t physical_unread_word08,
    std::uint32_t physical_unread_word0c, const void* source);
