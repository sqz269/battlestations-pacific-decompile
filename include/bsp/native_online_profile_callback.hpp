#pragma once

#include "bsp/native_online_notifications.hpp"
#include "bsp/native_string.hpp"

#include <cstdint>

namespace bsp {

// Borrow the application's real publication cells. The current game points to
// a previously constructed game owner with a live profile at +650h and a name
// mirror at +1FF0h. The pool context carries the actual 01090AA8/AA4/AA0
// cells, not a second pool or a projected profile/manager copy.
struct NativeOnlineProfileCallbackContext final {
    NativeOnlineManagerStorage* volatile& current_manager_00f8abe8;
    void* volatile& current_game_00e188a8;
    NativeStringRawPoolContext& pool;
};

// Full A3EAE0: native ECX=captured actual manager, EAX=name pointer, RET.
// Use DWORD wrapping for selected index <<7 and +90h. The returned pointer
// must address a readable C string; no selected-user range is invented here.
const char* selected_native_online_name_00a3eae0(
    const NativeOnlineManagerStorage& captured_manager) noexcept;

// Full 7F9340 and 7F9290 normal bodies over the actual profile at game+650h.
// Native ECX=profile, stack pointer to an eight-byte NativeString header,
// RET4. The source interface additionally supplies the current game publication
// for the separate +1FF0 mirror reload inside each setter, and the actual pool.
// Header +50h is display name, +3Ch is base name. Both setters mirror +50h
// when nonempty, otherwise +3Ch, through a native 00469840 substring of 31
// bytes. No full profile owner or constructor is synthesized.
void set_native_profile_display_name_007f9340(void* captured_profile_650,
    const NativeString& source, void* volatile& current_game_00e188a8,
    NativeStringStorage& storage);
void set_native_profile_name_007f9290(void* captured_profile_650,
    const NativeString& source, void* volatile& current_game_00e188a8,
    NativeStringStorage& storage);

// Full 737D60 normal callback. Incoming ECX is ignored, no stack args, RET.
// Reload F8ABE8 for A3EAE0, then E188A8 independently before each setter.
// The temporary and both profile string headers use the same actual pool.
// Publication/lifetime stability across each synchronous call is required;
// a callback may replace publications between those calls. Native EH machine
// faults, invalid selected pointers and arbitrary asynchronous retirement are
// outside this C++ interface.
void apply_native_online_profile_name_00737d60(NativeOnlineProfileCallbackContext&);
// Focused composition entry with an already bound actual pool storage. The
// caller is responsible for matching it to context.pool; production runtime
// uses the overload above, which constructs that binding itself.
void apply_native_online_profile_name_00737d60(NativeOnlineProfileCallbackContext&,
    NativeStringStorage& actual_storage);

} // namespace bsp
