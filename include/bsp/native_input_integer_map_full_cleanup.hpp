#pragma once

#include <cstdint>

namespace bsp {
// ONLY admitted full-path prefixes 004CEF40..004CEFBE and
// 0055B230..0055B2AE (127B/49 instructions each). Ordinary range erasers and
// their partial/diagnostic paths remain incomplete. Native ECX tree, unused
// EDX; stacked output,first_owner,first_node,last_owner,last_node; RET14h,
// EAXoutput. Source keeps that input shape; this is not native ABI validation.
//
// Preconditions: actual valid owned tree/head, nonnull tree;
// first_owner==last_owner==tree, first_node==initial head.left,
// last_node==fresh head, with stable exclusive storage through selection.
// Output is the writable private8B iterator cell used by actual consumers.
// Raw TEST/CMP/load order is preserved; admitted full-path outcomes are
// preconditions, with no replacement bad-path guard/abort/stub/diagnostic.
// Owning actual subtree4C1890/55AF60 then CURRENT head reload/reset/output
// stores follow. Opaque tree+0 untouched; result captured before output writes.
void* __fastcall clear_native_input_int_only_tree_full_prefix_004cef40(
    void* tree, std::uint32_t unused_edx, void* output,
    void* first_owner, void* first_node, void* last_owner, void* last_node);
void* __fastcall clear_native_input_nested_int_tree_full_prefix_0055b230(
    void* tree, std::uint32_t unused_edx, void* output,
    void* first_owner, void* first_node, void* last_owner, void* last_node);

// Complete whole0055B490..0055B4C3 (52B/23 instructions), distinct from the
// historical guarded/cached-head projection. Native ECX actual12B header,
// EDXunused, no stacked inputs, plain RET and DEFINED EAX0 (pseudocode void).
// Capture head thenhead.left; exact owner/first/last arguments prove the full
// 55B230 path on stable owned storage. Private8B output; free CURRENT head,
// zero+4 then+8; opaque+0 untouched; returnDWORD0. No null/repeated-destroy guard.
std::uint32_t __fastcall destroy_native_input_nested_int_map_0055b490(
    void* tree, std::uint32_t unused_edx);

// All new APIs require finite uniquely owned readable actual nodes/sentinels
// from matching singleton_lifetime_free current-CRT allocations. No callback,
// NativeStringStorage, RawPoolContext, allocator or invented node overlay.
// Owned scopes have no local EH registration/catch; producer EH is distinct
// evidence only. Source/private-helper frames and private output placement,
// register/flag residue, original heap identity, fault/SEH/stack exhaustion,
// original binary ABI and game behavior remain unverified. No whole ordinary
// partial eraser or InputSettings record readiness is promoted here.
} // namespace bsp
