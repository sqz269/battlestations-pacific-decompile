#pragma once
// cc13-pool22: 00525A10 BSP_UnitPickSelection_Construct, rebuilt over raw object memory.
// Every call the binary makes is a Pool22Host hook at the binary's point with its
// arguments. The STL vector erase (00524DC0) and _invalid_parameter_noinfo (00BF6713,
// the checked-iterator failure; it returns) are library code, bound with no credit.
// The name is a hypothesis. Reconstructed and build-tested only; the one caller,
// 00525A90 (src/cg_rebuild_b_cc13.cpp), still reaches it as the host hook
// CgRebuildBHost::container_init_00525a10.

#include <cstdint>

namespace bsp::pool22_cc13 {

struct Pool22Host {
    virtual ~Pool22Host() = default;
    // 00BF6713 _invalid_parameter_noinfo, cdecl, no arguments; returns.
    virtual void invalid_parameter_noinfo_00bf6713() = 0;
    // 00524DC0 STL_Vector_EraseRange, __thiscall(ECX = vector; ret*, first vector, first,
    // last vector, last) RET 14h: erase(first, last) of checked iterators {vector, ptr}.
    virtual void vector_erase_range_00524dc0(void* vector, void* ret, void* first_vector,
        void* first, void* last_vector, void* last) = 0;
};

// 00525A10, __thiscall(ECX = selection), plain RET, EAX = selection.
void* unit_pick_selection_construct_00525a10(void* selection, Pool22Host& host);

}  // namespace bsp::pool22_cc13
