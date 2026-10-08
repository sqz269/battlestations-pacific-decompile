#pragma once
#include <cstdint>

namespace bsp {

// Complete 0074E400 raw predicate: native ECX is the same actual plane,
// stack DWORD is the class word, full EAX is 0/1, RET4. EDX is unused.
// Nonconstant class words require the genuine live DWORD at plane+C4 and
// readable backing through C8. No copied class, semantic handle, profile
// substitute, default or virtual service is provided. Fixed native ancestor
// words return before the actual field read, in the original order.
// This raw entry does not bind the enclosing original class/table, constructor,
// world, lifetime, malformed/fault/concurrent access or game behavior.
std::uint32_t __fastcall native_plane_is_kind_0074e400(
    const void* actual_plane, void* unused_edx, std::uint32_t class_word);

} // namespace bsp
