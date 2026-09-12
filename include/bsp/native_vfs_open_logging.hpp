#pragma once

#include <cstdint>

namespace bsp {
class NativeStringStorage;
class ActualNativeStringPoolStorage;

// Borrow the actual publication slot and existing native pool lifetime domain.
// BDE9C0 only tests this slot for nonnull; it never dereferences its value.
struct NativeVfsOpenLoggingContext {
    void* volatile& file_log_0109cee8;
    ActualNativeStringPoolStorage& strings;
};

// Complete raw 18h builder producer: three actual8h string headers at +0/+8/+10.
// 4264A0 initializes empty, "%d", "%.3f"; 426500 calls it and returns this.
// Both original ABIs are ECX storage, RET, EAX storage. Existing contents are
// abandoned. Cleanup on failed construction follows the native EH states.
void* construct_native_log_builder_004264a0(void* actual_builder, NativeStringStorage&);
void* construct_native_log_builder_00426500(void* actual_builder, NativeStringStorage&);

// Complete appends into actual builder's first string. ECX destination,
// stacked source, RET4, EAX destination. BD1A20 snapshots source length and
// old destination length, then rereads both data pointers after resize.
// BD1A60 first copies nonnull C text to a pooled temporary, capturing its
// length/data before resize; null C text returns without touching destination.
void* append_native_log_header_00bd1a20(void* actual_builder,
    const void* actual_source_header, NativeStringStorage&);
void* append_native_log_cstring_00bd1a60(void* actual_builder,
    const char* source, NativeStringStorage&);

// Complete ECX builder, RET: releases current headers +10, +8, +0, preserving
// their bytes. There is no logger/sink call in this destructor or constructor.
void destroy_native_log_builder_00425f80(void* actual_builder,
    NativeStringStorage&) noexcept;

// Complete BDE9C0 body. Original ECX manager; stack name/unused-stream/mount
// DWORD; RET0C. Only mount's low byte is read. Allocates/searches/releases the
// dot/underscore temporaries BEFORE reading the gate, manager+79, and mount.
// The enabled nonsuppressed path constructs "<FILE><" + name + ">" and then
// destroys it. Verified installed instructions have NO output/registration
// operation: no synthetic sink is supplied. New C++ API, no original SEH/ABI
// replacement or game validation. See docs/NATIVE_VFS_OPEN_LOGGING.md.
void log_native_vfs_opened_resource_00bde9c0(void* actual_manager,
    const void* actual_name, void* unused_stream, std::uint32_t mount_byte,
    NativeVfsOpenLoggingContext&);
} // namespace bsp
