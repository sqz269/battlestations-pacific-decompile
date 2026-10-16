#pragma once

#include "bsp/native_string.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Actual scene class-name lookup requires MSVC Win32.
#endif

namespace bsp {

// Stable/disjoint binding metadata for distinct native frame regions. At
// original entry ESP=P: live parameter4=P+4, key header8=P-20h, iterator12=P-18h.
// No caller initialization of the header or iterator is invented. Bind actual
// reached backing; Source/native private-stack coincidence is not supplied.
struct NativeSceneClassNameLookupFrame {
    const void* actual_parameter4;
    void* actual_key_header8;
    void* actual_iterator12;
};

// Complete00469690..004697A4 [277B/94ops]. Original ECX scene database,
// stacked class-name pointer, RET4/EAX class-id DWORD or0. Bind the actual NUL
// fallback role00E18560, not a new owner or copied string. Real raw iterator,
// header ctor, raw pool getter/return and current SDK stricmp are composed.
// This additional-metadata C++ interface is not the original register ABI.
std::uint32_t find_native_scene_class_id_00469690(void* actual_scene_database,
    const NativeSceneClassNameLookupFrame&, NativeStringRawPoolContext&,
    const char* actual_empty_00e18560);

// Begin runs before the late input cell read/ctor. Capture current iterator
// node, header.data, then header.length ONCE after ctor. Reuse cached string
// values throughout comparison and explicit normal release; do not reread the
// header or add RAII cleanup. The entire body leaves its state at-1: the state0
// action exists but is never armed by any instruction in this normal body.
// Capture mapped descriptor/id before release, and discard Next EAX before
// reloading CURRENT iterator.node. No count/clamp/owner validation is added.
// Valid reached raw/alias storage, C strings, actual pool lifetimes and current
// CRT policy are required. Original private/frame-state aliases,FH3/SEH,fault,
// register/flags/mangling/binary ABI,native differential and game proof are open.

} // namespace bsp
