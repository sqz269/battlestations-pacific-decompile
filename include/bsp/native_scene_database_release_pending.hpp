#pragma once

#include "bsp/native_string.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Raw scene database release requires MSVC Win32.
#endif

namespace bsp {

// Complete0046BD10..0046BE41. Native ECX database, RET/no semantic result.
// Walk tree db+18h deleting nonnull mapped objects WITHOUT zeroing their slots;
// then walk pending pointer vector db+24h, likewise without clearing slots.
// Destroy/reset the original captured map, then erase the current vector range
// with the native wrapped/signed suffix arithmetic and real current memmove_s.
// All returnable current-CRT validation branches remain, except the unreachable
// native CMP ESI,ESI diagnostic (its full listing is documented).
//
// actual_private12 is stable writable backing: saved_database+0 and actual8h
// iterator {owner+4,node+8}. Initial values are not read. The saved database is
// reread after the map walk; callbacks can affect later vector selection through
// valid payload aliases. Initial node is captured BEFORE these local stores.
// Actual objects require callable Win32 __thiscall deleting vtable slot0(flag1).
// Private backing, raw pool metadata and reached storage remain valid through
// handlers, virtual calls and cleanup. Original physical cross-frame aliases,
// registers, CRT/heap/exception ABI and game/application behavior are unproved.
// No new host callback, ownership adapter, admission check or rollback is added.
void release_native_scene_database_records_and_pending_0046bd10(
    void* actual_database, void* actual_private12, NativeStringRawPoolContext&);

} // namespace bsp
