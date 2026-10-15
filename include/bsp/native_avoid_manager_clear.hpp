#pragma once

#include "bsp/cc13_avoid1.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native avoidance manager clear requires MSVC Win32.
#endif

namespace bsp {
// Complete 0041F600..0041F639 over the existing manager storage. First zero
// manager+58h, then inspect all twenty slots +08h..+54h. For each nonnull group,
// dispatch its CURRENT vtable[0] with ECX group and stacked flags1. Only after
// that call returns, null the SAME slot. Later slots are reloaded at their use.
// Then tail-compose the concrete full cache cleanup, not just the slot loop.
//
// Native: ECX manager, no stack inputs, plain tail JMP to0041E960. Source uses
// ECX manager and EDX the borrowed ADDRESS of the actual00E188A8 publication
// cell. The naked kernel saves that context once and POPs it back into EDX
// before tail-jumping into clear_native_avoid_cache_0041e960. No cached game
// pointer, substitute global, abstract deletion callback or shadow owner.
//
// Every actual vtable must select a compatible deleting entry and preserve
// its own calling convention. The matched Source group provider accepts ECX
// group, an ignored EDX placeholder (here the actual dispatch target), stacked
// flags, and RET4. Reached records, vtables, stack/control cells and provider
// state must stay valid. No local rollback or EH cleanup is added. The older
// avoid_zone_table_clear_0041f600 remains an explicitly partial semantic API.
// Current CRT/provider/Source-context identity and gameplay/native global
// binding remain qualified. Evidence: docs/GOAL15D_AVOID_MANAGER_CLEAR.md.
void __fastcall clear_native_avoid_manager_0041f600(
    Cc13AvoidZoneManagerNative* manager,
    void* const* actual_00e188a8_publication_cell);
} // namespace bsp
