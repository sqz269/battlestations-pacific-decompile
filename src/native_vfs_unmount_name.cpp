#include "bsp/native_vfs_unmount_name.hpp"

#include "bsp/native_adopted_substream.hpp"
#include "bsp/native_path_canonicalizer.hpp"
#include "bsp/native_physical_file_date.hpp"
#include "bsp/native_vfs_date_route.hpp"
#include "bsp/native_vfs_mount_records.hpp"
#include "bsp/native_vfs_mount_tree.hpp"

#include <exception>
#include <intrin.h>
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native VFS name unmount requires MSVC Win32.
#endif

namespace bsp {
namespace {
void* at(const void* owner, std::uint32_t offset) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<std::uintptr_t>(owner) + offset);
}
void* volatile& pointer(const void* owner, std::uint32_t offset = 0) noexcept {
    return *static_cast<void* volatile*>(at(owner, offset));
}
std::uint32_t word(const void* owner, std::uint32_t offset = 0) noexcept {
    return *static_cast<const volatile std::uint32_t*>(at(owner, offset));
}
void invalid(const SingletonLifetimeCallbacks& callbacks) {
    callbacks.invalid_parameter(callbacks.context);
}
} // namespace

struct NativeVfsUnmountNameAcquired::Impl {
    NativeVfsUnmountNamePhase phase = NativeVfsUnmountNamePhase::fresh;
    std::uint32_t active = 0;
    std::uint32_t failed_at = 0;
    std::uint32_t normalized[2]{};
    void* iterator[2]{};
    void* tree = nullptr;
    void* captured_owner = nullptr;
    void* captured_node = nullptr;
    void* captured_head = nullptr;
    void* captured_provider = nullptr;
    void* captured_table = nullptr;
    std::uint32_t captured_entry = 0;
    void* release_data = nullptr;
    std::uint32_t release_bytes = 0;
    NativeStringPoolStorage* release_pool = nullptr;
};

NativeVfsUnmountNameAcquired::NativeVfsUnmountNameAcquired()
    : impl_(std::make_unique<Impl>()) {}
NativeVfsUnmountNameAcquired::~NativeVfsUnmountNameAcquired() {
    if (impl_->phase != NativeVfsUnmountNamePhase::fresh &&
        impl_->phase != NativeVfsUnmountNamePhase::complete) std::terminate();
}
NativeVfsUnmountNamePhase NativeVfsUnmountNameAcquired::phase() const noexcept { return impl_->phase; }
std::uint32_t NativeVfsUnmountNameAcquired::active_call_site() const noexcept { return impl_->active; }
std::uint32_t NativeVfsUnmountNameAcquired::failure_site() const noexcept { return impl_->failed_at; }
const void* NativeVfsUnmountNameAcquired::actual_normalized_header() const noexcept { return impl_->normalized; }
const void* NativeVfsUnmountNameAcquired::actual_iterator() const noexcept { return impl_->iterator; }

bool unmount_native_vfs_name_00be0750(void* manager, const void* system_name,
    const void* mount_prefix, NativeVfsUnmountNameContext& context,
    NativeVfsUnmountNameAcquired& acquired) {
    auto& frame = *acquired.impl_;
    if (frame.phase != NativeVfsUnmountNamePhase::fresh) {
        throw std::logic_error("Native VFS unmount frame cannot replay");
    }
    const auto finish = [&](bool result) {
        frame.phase = NativeVfsUnmountNamePhase::releasing_name;
        // Both native exits disarm state0 BEFORE the getter. Capture data and
        // wrapping length+1 before it; leave the actual header untouched.
        frame.release_data = pointer(frame.normalized, 4);
        if (frame.release_data) {
            frame.release_bytes = word(frame.normalized) + 1u;
            frame.active = result ? 0x00be08b5u : 0x00be08f3u;
            frame.release_pool = context.canonicalizer.services.string_pool_00419cc0();
            frame.active = result ? 0x00be08bcu : 0x00be08fau;
            context.canonicalizer.services.return_string_00bd1510(frame.release_pool,
                frame.release_data, frame.release_bytes, 1);
        }
        frame.active = 0;
        frame.phase = NativeVfsUnmountNamePhase::complete;
        return result;
    };
    try {
        frame.phase = NativeVfsUnmountNamePhase::canonicalizing;
        frame.active = 0x00be0776;
        canonicalize_native_path_00bee390(frame.normalized, mount_prefix, context.canonicalizer);
        frame.phase = NativeVfsUnmountNamePhase::trimming;
        frame.active = 0x00be078c;
        trim_native_string_right_00584110(frame.normalized, context.trim_set_00ce7898,
            context.canonicalizer.strings);
        frame.tree = at(manager, 0x3c);
        frame.captured_node = pointer(pointer(manager, 0x40));
        frame.captured_owner = frame.tree;
        pointer(frame.iterator, 4) = frame.captured_node;
        pointer(frame.iterator) = frame.captured_owner;
        for (;;) {
            frame.phase = NativeVfsUnmountNamePhase::searching;
            // EDI captures the current end before the returning owner check.
            // EBX/ESI remain captured even if a handler mutates the iterator.
            frame.captured_head = pointer(frame.tree, 4);
            if (!frame.captured_owner || frame.captured_owner != frame.tree) {
                frame.active = 0x00be07ae;
                invalid(context.invalid_parameters);
            }
            if (frame.captured_node == frame.captured_head) return finish(false);
            if (!frame.captured_owner) {
                frame.active = 0x00be07bf;
                invalid(context.invalid_parameters);
            }
            if (frame.captured_node == pointer(frame.captured_owner, 4)) {
                frame.active = 0x00be07c9;
                invalid(context.invalid_parameters);
            }
            frame.active = 0x00be07fe;
            if (equal_native_string_headers_00435c40(
                at(pointer(frame.captured_node, 0x18), 8), system_name)) {
                if (frame.captured_node == pointer(frame.captured_owner, 4)) {
                    frame.active = 0x00be0814;
                    invalid(context.invalid_parameters);
                }
                frame.active = 0x00be0842;
                if (equal_native_string_headers_00435c40(
                    at(frame.captured_node, 0x10), frame.normalized)) {
                    if (frame.captured_node == pointer(frame.captured_owner, 4)) {
                        frame.active = 0x00be086e;
                        invalid(context.invalid_parameters);
                    }
                    frame.phase = NativeVfsUnmountNamePhase::releasing_provider;
                    frame.captured_provider = pointer(frame.captured_node, 0x18);
                    frame.active = 0x00be087a;
                    if (_InterlockedDecrement(static_cast<volatile long*>(
                        at(frame.captured_provider, 4))) == 0) {
                        frame.captured_table = pointer(frame.captured_provider);
                        frame.captured_entry = word(frame.captured_table);
                        frame.active = 0x00be088a;
                        context.providers.source_zero_reference(frame.captured_entry,
                            frame.captured_provider,
                            reinterpret_cast<std::uintptr_t>(frame.captured_table));
                    }
                    frame.phase = NativeVfsUnmountNamePhase::erasing;
                    frame.active = 0x00be0895;
                    erase_native_vfs_mount_iterator_00be0080(frame.tree, frame.iterator,
                        frame.captured_owner, frame.captured_node,
                        context.canonicalizer.strings, context.invalid_parameters);
                    return finish(true);
                }
            }
            frame.active = 0x00be0857;
            advance_native_vfs_mount_iterator_00bd97e0(frame.iterator, context.invalid_parameters);
            frame.captured_node = pointer(frame.iterator, 4);
            frame.captured_owner = pointer(frame.iterator);
        }
    } catch (...) {
        // Unknown nested providers/CRT/pool failures may still borrow this
        // storage. Retain it and the exact site; native FH3 is not simulated.
        frame.failed_at = frame.active;
        frame.phase = NativeVfsUnmountNamePhase::failed;
        throw;
    }
}
} // namespace bsp
