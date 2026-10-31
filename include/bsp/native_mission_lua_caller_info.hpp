#pragma once

#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native mission Lua caller info requires MSVC Win32.
#endif

struct lua_Debug;

namespace bsp {

// Whole game-owned 006B86B0, ordinary Source API. Capture level/record values,
// read CURRENT machine+4 and call real lua_getstack. Exactly zero returns zero
// immediately; ANY nonzero rereads CURRENT machine+4, calls real lua_getinfo
// with the SAME captured record, and returns its actual full int unchanged.
//
// actual_source_record must be a genuinely live public linked-Source lua_Debug
// object, not Native bytes cast to that type. No record birth/zeroing/default
// or output cleanup is performed here. actual_nSlu_00cf8398 borrows precisely
// the actual immutable mapped 5B query role at 00CF8398, including NUL; caller
// supplies mapping/content/extent provenance. It is not query customization,
// a semantic copied literal, new global or an early query-payload read.
//
// Caller retains raw machine backing, reached Source states/frames/code and
// record/query lifetimes, serialization and compatible mutation. The first
// state's i_ci must remain valid for whichever CURRENT state is reread; no
// state equality guard/cache/repair is added. Record string pointers remain
// borrowed through their own consumer intervals. No model cast, owner/Frame,
// capacity check, catch/pcall, cleanup, retry or post-call payload read.
// Original private interpreter/debug ABI/homes/FH3/SEH/fault/runtime and
// parent/factory/VM/startup are unproved; no vendor body is reconstructed.
std::int32_t get_caller_info_native_mission_lua_006b86b0(
    const void* actual_machine, std::int32_t level,
    lua_Debug* actual_source_record, const char* actual_nSlu_00cf8398);

} // namespace bsp
