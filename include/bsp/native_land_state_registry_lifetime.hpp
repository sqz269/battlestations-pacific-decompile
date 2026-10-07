#pragma once

#include "bsp/native_land_state_registry.hpp"

namespace bsp {

// Complete ordinary SOURCE bodies, with new C++ interfaces rather than original
// ECX/RET bridges. Require nonnull stable live actual storage, coherent vector
// headers, and any nonnull begin allocation in the existing
// singleton_lifetime_allocate/free CRT domain. No structural reentry, concurrent
// mutation, fault or private-EH admission. Pair names, state pointees and proxy
// are borrowed and untouched. No element destructors or callback providers.
//
// Numeric CE37DC is the original one-slot table identity, NOT a callable SOURCE
// profile. Its sole original entry is 411810; these direct helpers do not make
// the image pointer callable or bind the original class ABI/lifetime/game.
void release_native_bot_state_vector_00411610(NativeBotStateVectorStorage&);
void destroy_native_bot_state_registry_004116d0(NativeBotStateRegistryStorage&);

// Original: stack DWORD flags, TEST low byte bit0 AFTER buffer free and BEFORE
// three header clears, EAX original identity, RET4. Flag0 leaves the receiver
// alive. Flag1 additionally requires a separately allocated actual registry in
// the SAME CRT domain; an embedded approach/task member must not be self-freed.
// Returned flag1 identity is dangling and must not be dereferenced. No transfer
// of borrowed proxy/name/state ownership, native allocator/EH or ABI claim.
NativeBotStateRegistryStorage* scalar_delete_native_bot_state_registry_00411810(
    NativeBotStateRegistryStorage&, std::uint32_t flags);

} // namespace bsp
