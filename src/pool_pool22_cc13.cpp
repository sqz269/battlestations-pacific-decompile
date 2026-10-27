// cc13-pool22: 00525A10 rebuilt from the exported listing (bsp.py show 00525A10 --asm, read
// instruction by instruction on 2026-10-24). See include/bsp/pool_pool22_cc13.hpp.

#include "bsp/pool_pool22_cc13.hpp"

namespace bsp::pool22_cc13 {
namespace {

std::uint8_t* at(void* p, std::uint32_t offset) { return static_cast<std::uint8_t*>(p) + offset; }
template <class T> T& field(void* p, std::uint32_t offset) { return *reinterpret_cast<T*>(at(p, offset)); }

}  // namespace

// ---------------------------------------------------------------------------
// 00525A10 BSP_UnitPickSelection_Construct (45 insns)
// ABI: __thiscall(ECX = selection), no stack arguments, plain RET, EAX = selection
// (00525A7B). Only caller 00525A90 Unknown_VtCECCF8_Construct.
// Layout: +0 unit (0), +4 a std::vector {+4 allocator, +8 first, +0Ch last, +10h end},
// +14h index. Steps: the vector's first/last/end and +0 are zeroed (00525A37..00525A40);
// EH state 0 (handler 00C6B52B) covers the vector; the two checked-iterator tests for
// begin() and end() (first > last -> 00BF6713, 00525A45 and 00525A56) run on the
// re-read fields; then 00524DC0(ECX = vector; &ret, vector, first, vector, last), i.e.
// clear() on the just-emptied vector (pushes 00525A60..00525A68, last pushed first);
// +14h = -1. The returned iterator lands in a stack temporary that is not read.
// EH: no host call can throw into a partly built state other than the vector, whose
// unwind (the funclet's vector destructor) is not modelled.
// ---------------------------------------------------------------------------
void* unit_pick_selection_construct_00525a10(void* selection, Pool22Host& host) {
    void* vector = at(selection, 0x4);
    field<void*>(vector, 0x4) = nullptr;
    field<void*>(vector, 0x8) = nullptr;
    field<void*>(vector, 0xC) = nullptr;
    field<void*>(selection, 0x0) = nullptr;
    void* const last = field<void*>(vector, 0x8);
    if (field<std::uintptr_t>(vector, 0x4) > reinterpret_cast<std::uintptr_t>(last)) {
        host.invalid_parameter_noinfo_00bf6713();
    }
    void* const first = field<void*>(vector, 0x4);
    if (reinterpret_cast<std::uintptr_t>(first) > field<std::uintptr_t>(vector, 0x8)) {
        host.invalid_parameter_noinfo_00bf6713();
    }
    std::uint32_t ret[2]{};
    host.vector_erase_range_00524dc0(vector, ret, vector, first, vector, last);
    field<std::int32_t>(selection, 0x14) = -1;
    return selection;
}

}  // namespace bsp::pool22_cc13
