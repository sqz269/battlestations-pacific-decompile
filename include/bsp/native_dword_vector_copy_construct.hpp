#pragma once

#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Actual DWORD vector copy construction requires MSVC Win32.
#endif

namespace bsp {
// Whole00557590:196B/70instructions. Native ECX destination,incomingEDX unused;
// ONE stacked source header,RET4/EAXoriginaldestination. Actual16B header has
// opaque0/begin4/end8/capacityC;elements are unowned four-byte DWORDs.
// Capture sourcecount BEFORE clearing destination4/8/C. Count0 returns early.
// Actual length/allocation happen after clears but OUTSIDE the protected scope.
// Publish allocated begin/end/capacity;capture CURRENT sourceend and compare
// CURRENT sourcebegin BEFORE state0. Retain savedfirst across the second
// returning diagnostic;reload destinationbegin AFTER that report. Actual555000
// copies progressively;publish its advanced output,not a count-derived result.
void* __fastcall construct_native_checked_dword_vector_copy_00557590(
    void* destination, std::uint32_t unused_edx, const void* source);

// Native557654 is represented ONLY by the lexical catch in the routine above:
// tidy CURRENT originaldestination via actual4CB130,then barethrow. It is not
// a public standalone function or a fabricated original EBP/FH3 frame. No saved
// allocation free,rollback,initial old-backing free,selfalias repair,Buy call,
// callback,provider,object overlay or new allocator is added.
//
// All reached raw accesses/releases and child equality termination must be
// valid;destination stays writable through cleanup. Public aliases retain the
// specified captures/current reloads;private Source argument/frame aliases and
// asynchronous mutation are excluded. Current CRT allocation/free/new-handler
// and accepted owning C++ length-error transport apply. Original FH3/RTTI/
// private-frame/fault/SEH/heap/CRT/ambient-register/flag/binaryABI/game identity
// are unproved. Matching formals do not establish the original native ABI.
// Correct library names and old qualified APIs remain intact;new Source names
// are hypotheses. No placement-wrapper/action or broader-parent Source here.
} // namespace bsp
