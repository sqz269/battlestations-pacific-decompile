#pragma once

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native hull list head requires MSVC Win32.
#endif

namespace bsp {

struct Cc13AvoidZoneHullListNode;

// Complete 0041BF50..0041BF69: real raw16/16 allocation, guarded next/previous
// self-link stores, EAX head, plain RET. No receiver or stack arguments.
// Node words +8h/+Ch remain untouched/uninitialized. No hull handle constructor,
// value initialization, payload class or owner lifetime is introduced.
// Computed field-address guards and DWORD wrap are retained by the naked entry.
Cc13AvoidZoneHullListNode* __cdecl buy_native_hull_list_head_0041bf50();

// Complete 0041E1B0..0041E1C5: ECX existing raw0Ch header, call the real head
// producer first, write header+4=head then header+8=0, return header in EAX.
// Header+0 is untouched; ESI preserved; no stack arguments and plain RET.
void* __fastcall construct_native_hull_list_0041e1b0(void* actual_list_header);

// Source uses the existing Cc13 hull-node schema and genuine current CRT
// singleton_lifetime_allocate object16/16 provider. Current heap/new-handler/
// exception identity, external provider frames, original binding and gameplay
// remain qualified. No fake allocator, extra null-return/rollback or failure
// policy. Reached header/link storage must be valid; payload bytes are unread.
// Evidence: docs/GOAL15E_HULL_LIST_HEAD.md.

} // namespace bsp
