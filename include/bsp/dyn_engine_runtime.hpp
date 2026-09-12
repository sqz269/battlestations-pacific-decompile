#pragma once

#include "bsp/avoid_zone_dyn_hull.hpp"

namespace bsp {
struct alignas(4) DynProfileNodeStorage { unsigned char bytes[0x48]; };
struct alignas(4) DynProfileStorage { unsigned char bytes[0x9c]; };
struct alignas(4) DynEngineStorage { unsigned char bytes[0x14]; };

// Borrow the same publication cells used by world/scene construction. Root's
// native string is at 00D79DC8; its bytes are copied through the existing legacy
// string implementation. No replacement task manager or global is introduced.
struct DynEngineRuntimeContext {
    AvoidZoneDynHullMemory memory;
    void** engine_slot_0109e9fc;
    void** profile_slot_0109e9f8;
    const char* root_name_00d79dc8;
};

// 00C44000..00C44081. Native EDX=name, stack(node*, word2C), EAX=node, RET8.
// Complete normal storage construction; reuses NativeLegacySboStringStorage.
// Node+00/+10/+44 and the unused inline-string bytes remain untouched. The
// arbitrary word at+2C is preserved, without inferring a new ownership rule.
DynProfileNodeStorage* dyn_profile_node_construct_00c44000(
    DynProfileNodeStorage&, const char* name, std::uint32_t word_2c);

// 00C50310..00C50382. Native ESI=profile, EAX=profile, plain RET.
// Allocates the actual48h Root node, publishes the profile after its three head
// words, then clears the90h tail. It does not initialize the engine global.
DynProfileStorage* dyn_profile_construct_00c50310(
    DynProfileStorage&, const DynEngineRuntimeContext&);

// 00C55EA0..00C55F4E. Native stack(engine*,descriptor*), EAX=engine, RET8.
// Fresh14h engine owns its profile and real358h task manager. Descriptor points
// to its consumed first dword; it is read only after task-manager allocation.
DynEngineStorage* dyn_engine_construct_00c55ea0(DynEngineStorage&,
    const std::uint32_t* descriptor_worker_count, const DynEngineRuntimeContext&);

// 00C55F50..00C55FBE. Native ECX=descriptor*, EAX=existing/new engine, RET.
// Existing nonnull publication returns immediately. Otherwise publish only
// after the complete constructor returns. The native allocator throws on
// exhaustion; explicit null-return branches are retained, but native EH cleanup,
// destruction, profile timing and task execution are separate responsibilities.
// These are new C++ interfaces, not native ABI replacements. Inputs and borrowed
// cells must be valid for every path that reads them; no concurrent startup.
DynEngineStorage* dyn_engine_ensure_00c55f50(
    const std::uint32_t* descriptor_worker_count, const DynEngineRuntimeContext&);
} // namespace bsp
