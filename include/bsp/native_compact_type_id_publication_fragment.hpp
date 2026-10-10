#pragma once

#include <cstdint>

namespace bsp {

class TypeIdCounterLifetime;

// Ordinary Source interface for the retained, unowned CD867A..CD868D fragment:
// call the genuine shared counter provider, increment its returned next_id_04,
// then publish the old value to the actual current Compact word at 0109042C.
// This void interface makes no claim about Native arguments or return registers.
//
// The caller supplies the SAME genuine TypeIdCounterLifetime/process domain and
// actual borrowed current word. Their bindings, backing and lifetimes must remain
// valid/stable through the call; no private counter or copied word is substituted.
// Counter next_id_04 and actual_current_0109042c MAY alias: both ordered volatile
// stores are still required, and the final aliased word then contains the old ID.
// Neither cell may alias private Source objects, locals, or reference bindings.
// Access validity and the provider's existing contract remain caller obligations.
// No guard, descriptor layout, initializer owner, CRT/caller, or production
// backing is supplied. The observed Native body remains unowned.
void publish_native_compact_type_id_fragment_00cd867a(
    TypeIdCounterLifetime& same_counter_lifetime,
    volatile std::uint32_t& actual_current_0109042c);

} // namespace bsp
