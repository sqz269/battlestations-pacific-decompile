#pragma once

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native World activation flush requires MSVC Win32 assembly.
#endif

namespace bsp {

// Full 00903670..009036A8: ECX actual World-like receiver, no stack arguments,
// ESI preserved, plain RET. The descriptive name is provisional.
// Capture receiver+4 header then header.first once. A null first pointer is
// empty; the header itself must be readable. No full object layout is implied.
// Gate each current entity on byte5E != 0, full DWORD6C == 0, and parent3C null
// or parent byte5E == 0, in that order. Call the concrete destroy-state helper,
// then read that same entity's current+38 successor after its complete return.
// Caller supplies actual receiver/header/entity/parent storage and the helper's
// real callable+84/hierarchy contract. Every current entity must survive its
// post-call+38 read; a parent must survive its reached byte5E read. Header/first
// are never reloaded. Compatible callee-saved-register/stack return is required.
// Cycles can loop. Invalid storage can fault with preceding callee effects
// retained. No ownership, guards, recovery or new exception policy is supplied.
void __fastcall flush_native_world_activations_00903670(void* actual_world);

} // namespace bsp
