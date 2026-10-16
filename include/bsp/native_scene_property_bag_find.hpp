#pragma once

#include "bsp/native_string.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Bounded raw scene-property lookup requires MSVC Win32.
#endif

namespace bsp {

// Actual global roles, shared by every recursive call. Bind the real scratch
// buffer role with enough reached head-copy/terminator storage, distinct stable
// NUL fallback roles, and the application's existing pool/publication context.
// No capacity, private scratch copy, lock or new allocator domain is supplied.
struct NativeScenePropertyBagFindContext {
    void* actual_shared_head_00f89658;
    const char* actual_node_empty_00e177e4;
    const char* actual_query_empty_00e17654;
    NativeStringRawPoolContext& actual_string_pool;
};

// Entry P+4 actual writable key/bucket/partial-AL DWORD; actual temporary8B is
// P-14h..P-Dh. Only reached recursion needs the next stable metadata object and
// its actual cells. The parent writes tail into the child's actual parameter
// before recursion. Metadata/binding addresses stay stable and disjoint from
// described payload; no implicit Source/native private-stack aliases are made.
struct NativeScenePropertyBagFindFrame {
    void* actual_parameter4;
    void* actual_key_header8;
    const NativeScenePropertyBagFindFrame* recursive_frame;
};

// Whole375B/123-op CONTROL FLOW of008F2260, over BOUNDED actual providers.
// Native ECX bag, one live stacked key pointer, RET4/EAX property-or-zero.
// First dot uses actual shared scratch with overlap-capable copy. Map find
// writes the actual parameter DWORD; dot handling changes only its low byte,
// captures the mapped/partial word before cleanup and rereads CURRENT AL after
// cleanup. Type6 reads the nested bag POINTER from record+Ch before recursion.
// Constructor failure is unarmed. State0/1 guard arms only after construction,
// disarms before normal getter/release, and never retries an unarmed failure.
void* find_native_scene_property_bag_008f2260(void* actual_bag,
    const NativeScenePropertyBagFindFrame&, NativeScenePropertyBagFindContext&);

// This is NOT unrestricted raw/locale/alias Source-ready credit. Inherit the
// existing map provider's live108h map/finite correctly bucketed stable chain,
// closed matching NUL-free ASCII key/header, stable NUL fallback/current C
// locale and writable NONALIASING bucket-output admission. Every reached raw
// record, recursive frame, C string and pool/release must be valid for its real
// provider. Shared scratch/provider effects outside stable lookup are retained
// when those contracts remain valid; no extra reentry/concurrency guard or ban.
// General source_ready and unrestricted upstream admission remain unproved.
// Compiler cleanup actions CA4770/CA4778 map to the actual-header guard only,
// not new binary action entries or original RTTI/FH3/SEH/fault/register/CRT ABI.
// No model bag, std::string, callback, stub, input clamp or game proof is used.
} // namespace bsp
