#include "bsp/native_renderer_base_lifetime.hpp"

#include "bsp/native_diagnostic_sink_lifetime.hpp"
#include "bsp/native_render_service_base.hpp"
#include "bsp/native_renderer_worker_lifetime.hpp"
#include "bsp/native_singleton_publication.hpp"
#include "bsp/native_singleton_removal_reorder.hpp"
#include "bsp/native_singleton_vector_registration_wrappers.hpp"
#include "bsp/native_tracked_critical_section_release.hpp"
#include "bsp/singleton_lifetime.hpp"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <exception>

namespace bsp {
namespace {

struct NativeGuard final {
    std::uint32_t profile_00;
    CRITICAL_SECTION* section_04;
};
static_assert(sizeof(void*) == 4);
static_assert(sizeof(CRITICAL_SECTION) == 0x18);
static_assert(sizeof(NativeGuard) == 8);

std::byte* bytes(void* owner) noexcept { return static_cast<std::byte*>(owner); }

void store_dword(void* owner, std::size_t offset, std::uint32_t value) noexcept {
    std::memcpy(bytes(owner) + offset, &value, sizeof(value));
}

void* load_pointer(void* owner, std::size_t offset) noexcept {
    void* value;
    std::memcpy(&value, bytes(owner) + offset, sizeof(value));
    return value;
}

void store_pointer(void* owner, std::size_t offset, void* value) noexcept {
    std::memcpy(bytes(owner) + offset, &value, sizeof(value));
}

CRITICAL_SECTION* first_section(void* manager) noexcept {
    return static_cast<CRITICAL_SECTION*>(load_pointer(manager, 0x10));
}

volatile std::uint32_t& depth(CRITICAL_SECTION* section) noexcept {
    return *reinterpret_cast<volatile std::uint32_t*>(
        reinterpret_cast<std::byte*>(section) + 0x18);
}

NativeGuard enter_first_section(void* manager) {
    auto* const section = first_section(manager);
    NativeGuard guard{0x00ce37fcu, section};
    if (section) {
        EnterCriticalSection(section);
        depth(section) = depth(section) + 1u;
    }
    return guard;
}

struct GuardCleanup final {
    NativeGuard& guard;
    bool armed = true;
    ~GuardCleanup() noexcept {
        if (armed) {
            try { destroy_native_singleton_guard_00411ee0(&guard); }
            catch (...) { std::terminate(); }
        }
    }
};

void leave_first_section(GuardCleanup& cleanup) {
    destroy_native_singleton_guard_00411ee0(&cleanup.guard);
    cleanup.armed = false;
}

struct SubobjectRootCleanup final {
    void* subobject;
    bool armed = true;
    ~SubobjectRootCleanup() noexcept {
        if (armed) destroy_native_generic_singleton_base_00412430(subobject);
    }
};

struct PrimaryCleanup final {
    void* primary;
    bool armed = true;
    ~PrimaryCleanup() noexcept {
        if (armed) destroy_native_renderer_primary_base_00b33d90(primary);
    }
};

void release_primary_section(void* primary) noexcept {
    auto** const slot = reinterpret_cast<TrackedCriticalSection**>(bytes(primary) + 4);
    release_native_tracked_critical_section_0041cc80(slot);
}

} // namespace

void* construct_native_renderer_primary_base_00b33a70(void* primary) {
    store_dword(primary, 0, 0x00d5f1ecu);
    store_dword(primary, 4, 0);
    store_dword(primary, 8, 0);
    store_pointer(primary, 4, create_native_tracked_critical_section_00bd1860());
    return primary;
}

void destroy_native_renderer_primary_base_00b33d90(void* primary) noexcept {
    store_dword(primary, 0, 0x00d5f1ecu);
    release_primary_section(primary);
}

void* construct_native_renderer_singleton_subobject_00b25f40(
    void* subobject, NativeRendererBaseContext& context) {
    SubobjectRootCleanup base_cleanup{subobject}; // Native EH state0.
    store_dword(subobject, 0, 0x00d5e610u);
    void* const first_manager = get_native_singleton_manager_00415350(
        context.singleton_manager_01090aa0);
    NativeGuard guard = enter_first_section(first_manager);
    GuardCleanup guard_cleanup{guard}; // Native EH state1 after entry/depth.
    void* const primary = bytes(subobject) - 0x0c;
    context.current_renderer_00f8d394 = primary;
    void* const registered_subobject = primary ? bytes(primary) + 0x0c : nullptr;
    void* const second_manager = get_native_singleton_manager_00415350(
        context.singleton_manager_01090aa0);
    register_native_singleton_object_00bd0c30(
        second_manager, nullptr, registered_subobject);
    leave_first_section(guard_cleanup);
    base_cleanup.armed = false;
    return subobject;
}

void destroy_native_renderer_singleton_subobject_00b25fe0(
    void* subobject, NativeRendererBaseContext& context) {
    store_dword(subobject, 0, 0x00d5e610u);
    SubobjectRootCleanup base_cleanup{subobject}; // Native EH state0.
    void* const first_manager = get_native_singleton_manager_00415350(
        context.singleton_manager_01090aa0);
    NativeGuard guard = enter_first_section(first_manager);
    GuardCleanup guard_cleanup{guard}; // Native EH state1 after entry/depth.
    void* const current_primary = context.current_renderer_00f8d394;
    void* const current_subobject = current_primary ? bytes(current_primary) + 0x0c : nullptr;
    void* const second_manager = get_native_singleton_manager_00415350(
        context.singleton_manager_01090aa0);
    unregister_native_singleton_object_00bcfca0(
        second_manager, nullptr, current_subobject);
    context.current_renderer_00f8d394 = nullptr;
    leave_first_section(guard_cleanup);
    destroy_native_generic_singleton_base_00412430(subobject);
    base_cleanup.armed = false;
}

void* delete_native_renderer_singleton_subobject_00b26090(
    void* subobject, std::uint32_t flags, NativeRendererBaseContext& context) {
    void* const captured = subobject;
    destroy_native_renderer_singleton_subobject_00b25fe0(subobject, context);
    if ((flags & 1u) != 0) singleton_lifetime_free(captured);
    return captured;
}

void* construct_native_renderer_base_00b283f0(
    void* primary, NativeRendererBaseContext& context) {
    construct_native_renderer_primary_base_00b33a70(primary);
    PrimaryCleanup primary_cleanup{primary}; // Native EH state0.
    void* const subobject = bytes(primary) + 0x0c;
    construct_native_renderer_singleton_subobject_00b25f40(subobject, context);
    store_dword(subobject, 0, 0x00d5e76cu);
    store_dword(primary, 0, 0x00d5e628u);
    bytes(primary)[0x10] = std::byte{0};
    store_dword(primary, 0x18, 0x40000000u);
    primary_cleanup.armed = false;
    return primary;
}

void destroy_native_renderer_base_00b284e0(
    void* primary, NativeRendererBaseContext& context) {
    void* const subobject = bytes(primary) + 0x0c;
    store_dword(primary, 0, 0x00d5e628u);
    store_dword(subobject, 0, 0x00d5e76cu);
    PrimaryCleanup primary_cleanup{primary}; // Native EH state0.
    destroy_native_renderer_singleton_subobject_00b25fe0(subobject, context);
    primary_cleanup.armed = false; // Native state -1 before B33D90 call.
    destroy_native_renderer_primary_base_00b33d90(primary);
}

void* delete_native_renderer_base_00b28540(
    void* primary, std::uint32_t flags, NativeRendererBaseContext& context) {
    void* const captured = primary;
    destroy_native_renderer_base_00b284e0(primary, context);
    if ((flags & 1u) != 0) singleton_lifetime_free(captured);
    return captured;
}

void* delete_native_renderer_primary_base_00b33e10(
    void* primary, std::uint32_t flags) {
    void* const captured = primary;
    store_dword(primary, 0, 0x00d5f1ecu);
    release_primary_section(primary);
    if ((flags & 1u) != 0) singleton_lifetime_free(captured);
    return captured;
}

} // namespace bsp
