#pragma once

namespace bsp {

// Native ECX is the actual bucket-array/map base, NOT the manager base;
// stack is one borrowed raw8 key pointer, RET4, result is node or null.
// This ordinary Source interface supplies no Native ABI or loaded-owner proof.
// Caller retains actual map/key/chain/data and current CRT lifetime, serialized
// against invalidating writers. A returned node is borrowed, not a new owner.
void* find_native_locale_map_node_00a9ec70(
    const void* actual_map4004, const void* actual_key8);

} // namespace bsp
