#pragma once

#include <cstddef>
#include <cstdint>

namespace bsp {

inline constexpr std::size_t native_particle_model_payload_bytes = 0x2dc;
inline constexpr std::size_t native_particle_model_slot_bytes = 0x2e0;
inline constexpr std::size_t native_particle_model_slab_bytes = 0x5c44;

// AF5C20: ECX actual5C44h slab, stack slab index; EAX same slab; RET4.
// Complete producer: initialize32 slot IDs and free indices31..0, preserving
// every2DCh payload and final padding WORD+5C42. No model construction.
void* initialize_native_particle_model_slab_00af5c20(
    void* actual_slab, std::uint32_t slab_index) noexcept;

// AF69E0: ECX actual initialized38h pool; EAX actual2E0h raw slot; RET.
// Complete allocation body. Borrow real Win32 critical section+0C, depth+24,
// table+28, count+2C, capacity+30 and earliest+34. Grow through the existing
// shared CRT services. An allocation/new-handler exception retains lock,
// depth and prior publications; native code has no rollback or EH frame.
void* allocate_native_particle_model_slot_00af69e0(void* actual_pool);

// AF60B0: ECX actual initialized38h pool, stack raw slot; RET4. Complete
// raw return: read slot+2DC slab ID, divide signed wrapped32-bit displacement
// by736, push its WORD index, reload/increment free count, lower earliest
// using unsigned comparison. No model destruction or payload clearing.
void return_native_particle_model_slot_00af60b0(
    void* actual_pool, void* actual_raw_slot);

// AF6B60: overwrite incoming ECX (ignored size) with canonical00F8D2D0,
// then tail JMP AF69E0. The new source adapter borrows that actual owner.
void* allocate_native_particle_model_00af6b60(void* actual_pool_00f8d2d0);

// AF62F0: ECX raw slot; select00F8D2D0; CALL AF60B0; RET. Complete raw
// return thunk used by constructor unwind, without owner destruction.
void return_native_particle_model_00af62f0(
    void* actual_raw_slot, void* actual_pool_00f8d2d0);

// New MSVC Win32 source interfaces, not original ABI entry replacements.
// Every reached address requires valid backing and an initialized section.
// Actual pool startup, trim and destruction belong to native_particle_model_
// pool_owner.hpp; bind the genuine trim before allocator-list publication.
// Never substitute NativeModelPool (different global/188h slot) or zero storage.
// Physical AF74A0 model construction and gameplay are separate boundaries.
} // namespace bsp
