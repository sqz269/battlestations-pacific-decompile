#include "bsp/native_pak_registry_block_callbacks.hpp"

#include "bsp/native_pak_registry.hpp"
#include "bsp/native_pak_registry_block_helpers.hpp"
#include "bsp/native_path_canonicalizer.hpp"
#include "bsp/native_physical_file_date.hpp"
#include "bsp/native_string.hpp"
#include "bsp/native_vfs_device_route.hpp"
#include "bsp/native_vfs_mount_registration.hpp"
#include "bsp/native_vfs_name_resolution.hpp"
#include "bsp/native_vfs_unmount_name.hpp"

#include <cstring>
#include <exception>
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native PakRegistry block callbacks require MSVC Win32.
#endif

namespace bsp {
namespace {
using U = std::uint32_t;
void* at(const void* owner, U offset) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<std::uintptr_t>(owner) + offset);
}
void* volatile& pointer(const void* owner, U offset = 0) noexcept {
    return *static_cast<void* volatile*>(at(owner, offset));
}
volatile U& word(const void* owner, U offset = 0) noexcept {
    return *static_cast<volatile U*>(at(owner, offset));
}
std::int32_t signed_word(const void* owner, U offset = 0) noexcept {
    return static_cast<std::int32_t>(word(owner, offset));
}
} // namespace

struct NativePakRegistryBlockInvocation::Impl {
    NativePakRegistryBlockPhase phase = NativePakRegistryBlockPhase::fresh;
    U active = 0, failed_at = 0;
    U mpak[2]{}, device[2]{}, dot[2]{};
    NativeVfsNameResolutionAcquired resolver;
    NativeVfsUnmountNameAcquired unmount;
    void* registry = nullptr;
    void* vector = nullptr;
    void* provider = nullptr;
    void* slot = nullptr;
    U priority = 0, device_id = 0, index = 0, shift = 0, compared_length = 0;
    bool removed_mount = false;
    void* release_data = nullptr;
    U release_bytes = 0;
    NativeStringPoolStorage* release_pool = nullptr;

    void begin(void* owner) {
        if (phase != NativePakRegistryBlockPhase::fresh)
            throw std::logic_error("Native PakRegistry block invocation cannot replay");
        registry = owner;
    }
    void release_captured(void* header, NativePakRegistryBlockContext& context,
        U getter_site, U return_site) {
        if (!release_data) return;
        release_bytes = word(header) + 1u;
        active = getter_site;
        auto& services = context.mounting.canonicalizer.services;
        release_pool = services.string_pool_00419cc0();
        active = return_site;
        services.return_string_00bd1510(release_pool, release_data, release_bytes, 1);
    }
    void release(void* header, NativePakRegistryBlockContext& context,
        U getter_site, U return_site) {
        release_data = pointer(header, 4);
        release_captured(header, context, getter_site, return_site);
    }
    void finish() noexcept { active = 0; phase = NativePakRegistryBlockPhase::complete; }
    void failed() noexcept { failed_at = active; phase = NativePakRegistryBlockPhase::failed; }
};

NativePakRegistryBlockInvocation::NativePakRegistryBlockInvocation()
    : impl_(std::make_unique<Impl>()) {}
NativePakRegistryBlockInvocation::~NativePakRegistryBlockInvocation() {
    if (impl_->phase != NativePakRegistryBlockPhase::fresh &&
        impl_->phase != NativePakRegistryBlockPhase::complete) std::terminate();
}
NativePakRegistryBlockPhase NativePakRegistryBlockInvocation::phase() const noexcept { return impl_->phase; }
U NativePakRegistryBlockInvocation::active_call_site() const noexcept { return impl_->active; }
U NativePakRegistryBlockInvocation::failure_site() const noexcept { return impl_->failed_at; }
const void* NativePakRegistryBlockInvocation::actual_mpak_header() const noexcept { return impl_->mpak; }
const void* NativePakRegistryBlockInvocation::actual_device_header() const noexcept { return impl_->device; }
const void* NativePakRegistryBlockInvocation::actual_dot_header() const noexcept { return impl_->dot; }

void enter_native_pak_registry_block_00bb5770(void* registry, const void* name,
    NativePakRegistryBlockContext& context, NativePakRegistryBlockInvocation& invocation) {
    auto& f = *invocation.impl_;
    f.begin(registry);
    auto& strings = context.mounting.canonicalizer.strings;
    try {
        f.phase = NativePakRegistryBlockPhase::naming;
        f.active = 0x00bb5795;
        construct_native_pak_block_name_00bb5670(f.mpak, name, strings);
        f.active = 0x00bb57a7;
        set_native_vfs_block_active_00bd9210(context.actual_vfs_publication_0109ceec, 0);
        f.phase = NativePakRegistryBlockPhase::resolving;
        f.active = 0x00bb57b7;
        if (resolve_native_vfs_existing_name_00bdf4c0(
            context.actual_vfs_publication_0109ceec, f.mpak, context.resolution, f.resolver)) {
            f.phase = NativePakRegistryBlockPhase::copying_device_name;
            const auto length = word(f.mpak);
            word(f.device) = 0;
            word(f.device, 4) = 0;
            f.active = 0x00bb57d9;
            resize_native_string_header_0041dd40(f.device, strings, length, true);
            if (word(f.mpak) != 0) {
                const auto count = word(f.device);
                auto* const source = pointer(f.mpak, 4);
                auto* const destination = pointer(f.device, 4);
                f.active = 0x00bb57f3;
                if (count != 0) std::memmove(destination, source, count);
            }
            f.phase = NativePakRegistryBlockPhase::selecting_device;
            f.active = 0x00bb580e;
            f.device_id = static_cast<U>(select_native_vfs_device_00bdd850(
                context.actual_vfs_publication_0109ceec, f.device, f.device, context.resolution.device));
            word(registry, 0x0c) = word(registry, 0x0c) + 1u;
            f.priority = word(registry, 0x0c);
            f.phase = NativePakRegistryBlockPhase::constructing_dot;
            f.active = 0x00bb5824;
            construct_native_string_cstring_0041e870(f.dot, context.actual_dot_00ce3a70, strings);
            f.phase = NativePakRegistryBlockPhase::mounting;
            f.active = 0x00bb5842;
            f.provider = mount_native_vfs_system_path_00be1890(
                context.actual_vfs_publication_0109ceec, f.mpak, f.dot,
                f.priority, 0, f.device_id, context.mounting);
            f.vector = at(registry, 0x10);
            f.phase = NativePakRegistryBlockPhase::appending;
            const auto capacity = word(f.vector, 8);
            if (word(f.vector, 4) == capacity) {
                const auto doubled = capacity + capacity;
                const auto target = static_cast<std::int32_t>(doubled) > 1
                    ? static_cast<std::int32_t>(doubled) : 1;
                f.active = 0x00bb5860;
                reserve_native_pak_registry_slots_00bb46f0(f.vector, target);
            }
            const auto count = word(f.vector, 4);
            auto* const base = pointer(f.vector);
            f.slot = at(base, count * 4u);
            if (f.slot) pointer(f.slot) = f.provider;
            // BB5873 captures dot data BEFORE incrementing the current count.
            f.release_data = pointer(f.dot, 4);
            word(f.vector, 4) = word(f.vector, 4) + 1u;
            f.phase = NativePakRegistryBlockPhase::releasing_dot;
            f.release_captured(f.dot, context, 0x00bb588c, 0x00bb5893);
            f.phase = NativePakRegistryBlockPhase::updating_count;
            f.active = 0x00bb589e;
            increment_native_vfs_block_count_00bd9fa0(context.actual_vfs_publication_0109ceec);
            f.phase = NativePakRegistryBlockPhase::releasing_device_name;
            f.release(f.device, context, 0x00bb58ba, 0x00bb58c1);
        }
        f.phase = NativePakRegistryBlockPhase::recomputing;
        f.active = 0x00bb58cc;
        recompute_native_vfs_block_active_00bd9220(context.actual_vfs_publication_0109ceec);
        f.phase = NativePakRegistryBlockPhase::releasing_name;
        f.release(f.mpak, context, 0x00bb58eb, 0x00bb58f2);
        f.finish();
    } catch (...) { f.failed(); throw; }
}

void leave_native_pak_registry_block_00bb5910(void* registry, const void* name,
    NativePakRegistryBlockContext& context, NativePakRegistryBlockInvocation& invocation) {
    auto& f = *invocation.impl_;
    f.begin(registry);
    auto& strings = context.mounting.canonicalizer.strings;
    try {
        f.phase = NativePakRegistryBlockPhase::naming;
        f.active = 0x00bb5934;
        construct_native_pak_block_name_00bb5670(f.mpak, name, strings);
        f.phase = NativePakRegistryBlockPhase::resolving;
        f.active = 0x00bb594c;
        (void)resolve_native_vfs_existing_name_00bdf4c0(
            context.actual_vfs_publication_0109ceec, f.mpak, context.resolution, f.resolver);
        f.phase = NativePakRegistryBlockPhase::removing_entries;
        f.index = 0;
        if (signed_word(registry, 0x14) > 0) {
            f.compared_length = word(f.mpak);
            do {
                auto* const base = pointer(registry, 0x10);
                f.provider = pointer(at(base, f.index * 4u));
                const auto length = word(f.provider, 8);
                bool matches = length == f.compared_length;
                if (matches && length != 0) {
                    auto* const right = static_cast<const char*>(pointer(f.mpak, 4));
                    auto* const left = static_cast<const char*>(pointer(f.provider, 0x0c));
                    f.active = 0x00bb5992;
                    const auto comparison = _stricmp(left, right);
                    f.compared_length = word(f.mpak); // BB5997 reload, even on mismatch.
                    matches = comparison == 0;
                }
                if (matches) {
                    f.shift = f.index;
                    if (static_cast<std::int32_t>(f.shift) <
                        static_cast<std::int32_t>(word(registry, 0x14) - 1u)) {
                        do {
                            auto* const current_base = pointer(registry, 0x10);
                            auto* const slot = at(current_base, f.shift * 4u);
                            const auto next = word(slot, 4);
                            word(slot) = next;
                            const auto current_count = word(registry, 0x14);
                            f.shift += 1u;
                            if (static_cast<std::int32_t>(f.shift) >=
                                static_cast<std::int32_t>(current_count - 1u)) break;
                        } while (true);
                        f.compared_length = word(f.mpak); // Only the reached shift-loop exit reloads.
                    }
                    word(registry, 0x14) = word(registry, 0x14) - 1u;
                } else {
                    f.index += 1u;
                }
            } while (static_cast<std::int32_t>(f.index) < signed_word(registry, 0x14));
        }
        f.phase = NativePakRegistryBlockPhase::constructing_dot;
        word(f.dot) = 0;
        word(f.dot, 4) = 0;
        f.active = 0x00bb59f5;
        resize_native_string_header_0041dd40(f.dot, strings, 1, true);
        auto* const destination = pointer(f.dot, 4);
        if (destination) {
            const auto count = word(f.dot) + 1u;
            f.active = 0x00bb5a10;
            if (count != 0) std::memmove(destination, context.actual_dot_00ce3a70, count);
        }
        f.phase = NativePakRegistryBlockPhase::unmounting;
        f.active = 0x00bb5a2e;
        f.removed_mount = unmount_native_vfs_name_00be0750(
            context.actual_vfs_publication_0109ceec, f.mpak, f.dot, context.unmounting, f.unmount);
        f.phase = NativePakRegistryBlockPhase::releasing_dot;
        f.release(f.dot, context, 0x00bb5a4d, 0x00bb5a54);
        if (f.removed_mount) {
            f.phase = NativePakRegistryBlockPhase::updating_count;
            f.active = 0x00bb5a64;
            decrement_native_vfs_block_count_00bd9200(context.actual_vfs_publication_0109ceec);
            word(registry, 0x0c) = word(registry, 0x0c) - 1u;
        }
        f.phase = NativePakRegistryBlockPhase::recomputing;
        f.active = 0x00bb5a73;
        recompute_native_vfs_block_active_00bd9220(context.actual_vfs_publication_0109ceec);
        f.phase = NativePakRegistryBlockPhase::releasing_name;
        f.release(f.mpak, context, 0x00bb5a94, 0x00bb5a9b);
        f.finish();
    } catch (...) { f.failed(); throw; }
}
} // namespace bsp
