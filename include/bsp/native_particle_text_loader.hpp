#pragma once
#include <cstdint>

namespace bsp {
struct NativeStringRawPoolContext;
struct NativeVfsNameResolutionContext;
class NativeVfsNameResolutionAcquired;
class NativeVfsRuntimeBindings;

// Complete AF5850[221]. Native ECX actual1Ch TextBuffer; stacked nullable
// C-string; EAX actual-count/zero; RET4. This new C++ API borrows the SAME
// actual VFS publication, concrete VFS services and raw string-pool cells.
// name_resolution's lookup/logging strings must be the existing VFS owner
// service over those same cells. Its existing identity checks still apply.
// No new VFS owner, stream, table, callback or private publication is supplied.
//
// The caller supplies a FRESH invocation and retains it AND actual_text_buffer
// (including its name header/data) after failed resolution. The existing
// resolver deliberately terminates if an incomplete/failed frame is destroyed.
// This function creates no local acquired frame and does not change that policy.
// Other provider failures propagate with the original loader's lack of cleanup.
//
// Resolve the current name; ignore AL; reload publication for open(flags32h).
// Consume only low32 virtual30 length. Allocate that many bytes, make ONE
// current virtual24 read, with actual-count initialized from incoming name
// pointer bits. A mismatch frees CURRENT text+14 and clears it, returning0
// WITHOUT releasing the stream. Success decrements once and dispatches the
// CURRENT terminal only on zero, then reloads actual-count for the result.
// No null-stream recovery, read retry, length guard or rollback is added.
std::uint32_t load_native_particle_text_buffer_00af5850(
    void* actual_text_buffer, const char* name,
    void* volatile& actual_vfs_publication_0109ceec,
    NativeVfsRuntimeBindings& vfs,
    NativeVfsNameResolutionContext& name_resolution,
    NativeVfsNameResolutionAcquired& invocation,
    NativeStringRawPoolContext& strings);

// Native table words must remain readable at their identity addresses, as
// required by the existing concrete VFS bindings. Their supported profiles
// and source exception/CRT boundaries are unchanged. GameNativeVfsRuntime's
// private services are not exposed here; application wiring remains separate.
// Original argument-slot aliases, register ABI, hardware SEH and gameplay
// equivalence are not provided by this source interface.
} // namespace bsp
