#pragma once

#include <cstdint>

namespace bsp {

class TypeIdCounterLifetime;

// Ordinary Source interface for the retained, unowned CD8516..CD8529 fragment:
// call the genuine shared counter provider, increment its returned next_id_04,
// then publish the old value to the actual current SkinModel word at 01090344.
// This void interface makes no claim about Native arguments or return registers.
//
// The caller supplies the SAME genuine TypeIdCounterLifetime and actual borrowed
// current word. Their bindings, backing and lifetimes must remain valid/stable
// through the call; no private counter or copied current word is substituted.
// Counter next_id_04 and actual_current_01090344 MAY alias: both ordered volatile
// stores are still required, and the final aliased word then contains the old ID.
// Neither cell may alias private Source objects, locals, or reference bindings.
// Access validity and the provider's existing contract remain caller obligations.
// No guard, descriptor layout, initializer owner, CRT/caller, or production
// backing is supplied. The observed Native body remains unowned.
void publish_native_skin_model_type_id_fragment_00cd8516(
    TypeIdCounterLifetime& same_counter_lifetime,
    volatile std::uint32_t& actual_current_01090344);

} // namespace bsp
