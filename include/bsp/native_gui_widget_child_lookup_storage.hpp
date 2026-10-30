#pragma once
#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Actual raw Widget child lookup requires MSVC Win32.
#endif

namespace bsp {
// Whole AA7E00..AA7EB0 [177B/70ops]: ECX captured actual Widget,
// two physical DWORD argument homes, EAX borrowed child or null, RET8.
// Capture actual Widget+68 before reading home0 ONCE as the query-header
// pointer. Home1 is NEVER read: it is not a recursive flag in this body.
// Only direct children are searched; the parent's own name is never tested.
//
// Caller supplies the SAME genuine raw Widget/list/sentinel/sibling/payload/
// scene-node graph and actual query8B header/data, not GuiLayoutWidget or a
// typed owner reinterpretation. No header/string/list/count is copied. The
// query header is not dereferenced on an empty list or node-null-only path.
// Every reached positive-length name uses its actual terminated data pointers;
// no null-to-empty substitution, counted-span conversion or name policy exists.
//
// Head, payload, scene-node, header and data-pointer reads remain current at
// their Native sites. Captured iterator/query/list identities survive genuine
// SDK callbacks; handler mutations are observed only by reached reloads.
// Caller serializes the graph and retains every reached/captured backing through
// callbacks and return/escape. Current Page/Frame publication may change while
// the captured actual receiver remains valid. Returned child is an unretained
// borrow; its later lifetime belongs to the existing actual graph owner.
//
// Existing raw BF6713 invokes the selected real SDK invalid-parameter API and
// may return or throw. Comparison uses the same owning-SDK _stricmp as the raw
// 435C40 supplier, preserving this caller's distinct operand order. Original
// CRT locale/handler/Watson/private ABI/fault identity remains qualified.
// No new handler, catch, cleanup, count/retain, retry, rollback, terminal policy,
// recursion, cycle/iteration bound, owner, provider or process authority exists.
// This new ordinary C++ interface does not supply Original register/RET8/SEH/
// stack/public-alias/runtime proof or bind the actual Page/Widget application.
void* find_native_gui_widget_child_storage_00aa7e00(void* captured_actual_parent,
    volatile std::uint32_t (&actual_homes)[2]);
} // namespace bsp
