#pragma once

#include "bsp/singleton_lifetime.hpp"

#include <cstdint>

namespace bsp {

// Actual Win32 list storage: the manager's +7Ch is the list receiver, with
// sentinel at list+4 (manager+80h) and unsigned count at list+8 (+84h).
// Each 0Ch node contains next+0, previous+4 and one gate byte+8; the final
// three bytes are untouched. No separate host list or gate state is owned.

// 007F8390..007F83C2: incoming ECX unused, stack(next,previous,gate pointer),
// EAX allocated node, RET0C. The gate pointer is dereferenced after allocation.
// Preserves the individual native destination-address guards.
void* __stdcall allocate_native_fileblock_gate_node_007f8390(
    void* next, void* previous, const std::uint8_t* gate_slot);

// 007FA3A0..007FA430: ECX actual list, stack unsigned increment, EAX updated
// count, RET4. Bound is UINT32_MAX. Overflow throws the existing owning
// NativeAliasListLengthError transport without changing count. The caller
// allocates its node before this check; this helper owns no node rollback.
std::uint32_t grow_native_fileblock_gate_count_007fa3a0(
    void* actual_list, std::uint32_t increment);

// 00BDAF40..00BDAF9D: ECX list, stack(output,iterator owner,node), EAX output,
// RET0C. Returning BF6713 callbacks continue with captured owner/node. There
// is no owner==list check. Capture successor, unlink using current node links,
// free, decrement current list count, then store output+4 before output+0.
void* erase_native_fileblock_gate_iterator_00bdaf40(void* actual_list,
    void* output, void* iterator_owner, void* node,
    const SingletonLifetimeCallbacks& invalid_parameters);

// Descriptive names are hypotheses. Growth/erase expose new C++ interfaces;
// the callback and owning exception transport have host ABI/RTTI. The node
// producer preserves stack shape, but native binary/game parity is untested.
} // namespace bsp
