#include "bsp/native_input_backend_owner.hpp"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#define DIRECTINPUT_VERSION 0x0800
#include <Windows.h>
#include <dinput.h>
#include <cstring>
#include <stdexcept>

#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native input backend ownership requires MSVC Win32.
#endif

namespace bsp {
namespace {
template<class T> T read(const void* p, std::size_t offset) noexcept {
    T value; std::memcpy(&value, static_cast<const std::byte*>(p) + offset, sizeof value);
    return value;
}
template<class T> void write(void* p, std::size_t offset, T value) noexcept {
    std::memcpy(static_cast<std::byte*>(p) + offset, &value, sizeof value);
}
void* at(void* p, std::size_t offset) noexcept {
    return static_cast<std::byte*>(p) + offset;
}
void require_raw_lifetime(const NativeInputBackendOwnerContext& c) {
    if (!c.lifetime.uses_actual_storage())
        throw std::invalid_argument("native input backend requires the actual raw manager publication");
}
void root_profile(void* owner) noexcept { write<std::uint32_t>(owner, 0, 0x00ce3818); }
void destroy_classes(void* owner) noexcept {
    // BF7C6E walks the completed three elements in REVERSE address order.
    for (std::size_t index = 3; index != 0; --index)
        destroy_native_input_backend_class_00a90dc0(at(owner, 0x68 + (index - 1) * 0x24));
}
void unwind_groups(void* owner, NativeInputBackendOwnerContext& c) noexcept {
    // DEC8E4 state1 -> CB6628(classes), state0 -> CB6620(base).
    destroy_classes(owner);
    destroy_native_input_backend_base_00a90940(owner, c);
}
void unwind_backend(void* owner, NativeInputBackendOwnerContext& c) noexcept {
    // DECE7C state1 -> CB6A28(GUID vector), state0 -> CB6A20(groups).
    destroy_native_input_backend_guids_00a97b00(at(owner, 0xe4));
    destroy_native_input_backend_groups_00a90e00(owner, c);
}
void* allocate_object(std::size_t bytes) {
    return singleton_lifetime_allocate({SingletonAllocationKind::object, bytes, bytes});
}
} // namespace

void NativeInputBackendDirectInput::create_interface_00c2e016(void* module, void** output) {
    // Reserve BEFORE acquiring the SDK reference; insertion cannot allocate.
    references_.reserve(references_.size() + 1);
    create_result_ = DirectInput8Create(static_cast<HINSTANCE>(module), 0x800,
        IID_IDirectInput8A, output, nullptr);
    if (*output) references_.push_back(*output);
}
void NativeInputBackendDirectInput::add_ref_vslot04(void* actual_interface) {
    // Native null would fault at A98342. Explicit source contract error;
    // a nonnull creation output is still accepted regardless of HRESULT.
    if (!actual_interface)
        throw std::logic_error("native input backend DirectInput output is null before AddRef");
    references_.reserve(references_.size() + 1);
    static_cast<IDirectInput8A*>(actual_interface)->AddRef();
    references_.push_back(actual_interface);
}
void NativeInputBackendDirectInput::release_tracked_references() {
    // Host-only final boundary. Never called by a native owner destructor.
    while (!references_.empty()) {
        auto* const current = static_cast<IDirectInput8A*>(references_.back());
        references_.pop_back();
        current->Release();
    }
}

void* construct_native_input_backend_base_00a908a0(void* owner,
    NativeInputBackendOwnerContext& c) {
    require_raw_lifetime(c);
    write<std::uint32_t>(owner, 0, 0x00d5b5f4);
    try {
        CapturedSoundLifetimeSection section(c.lifetime);
        c.global_00f8bbf4 = owner;
        auto manager = c.lifetime.get_manager_00415350();
        manager->register_object(c.global_00f8bbf4);
    } catch (...) {
        // DEC87C: captured guard first, then 412430 profile reset. Preserve
        // publication, including failure before registration has completed.
        root_profile(owner);
        throw;
    }
    return owner;
}
void destroy_native_input_backend_base_00a90940(void* owner,
    NativeInputBackendOwnerContext& c) {
    require_raw_lifetime(c);
    write<std::uint32_t>(owner, 0, 0x00d5b5f4);
    try {
        CapturedSoundLifetimeSection section(c.lifetime);
        auto manager = c.lifetime.get_manager_00415350();
        manager->unregister_object(c.global_00f8bbf4);
        c.global_00f8bbf4 = nullptr;
    } catch (...) {
        // DEC8B0: release captured guard before resetting this owner's root.
        root_profile(owner);
        throw;
    }
    root_profile(owner);
}
void* scalar_delete_native_input_backend_base_00a909e0(void* owner,
    std::uint8_t flags, NativeInputBackendOwnerContext& c) {
    destroy_native_input_backend_base_00a90940(owner, c);
    if ((flags & 1u) != 0) singleton_lifetime_free(owner);
    return owner;
}

void* construct_native_input_backend_class_00a914c0(void* group) noexcept {
    write<std::uint32_t>(group, 8, 0); write<std::uint32_t>(group, 0xc, 0);
    write<std::uint32_t>(group, 0x10, 0); write<std::uint32_t>(group, 0x18, 0);
    write<std::uint32_t>(group, 0x1c, 0); write<std::uint32_t>(group, 0x20, 0);
    return group;
}
void destroy_native_input_backend_class_00a90dc0(void* group) noexcept {
    if (void* const second = read<void*>(group, 0x18)) singleton_lifetime_free(second);
    write<std::uint32_t>(group, 0x18, 0); write<std::uint32_t>(group, 0x1c, 0);
    write<std::uint32_t>(group, 0x20, 0);
    if (void* const first = read<void*>(group, 8)) singleton_lifetime_free(first);
    write<std::uint32_t>(group, 8, 0); write<std::uint32_t>(group, 0xc, 0);
    write<std::uint32_t>(group, 0x10, 0);
}
void* construct_native_input_backend_groups_00a91570(void* owner,
    NativeInputBackendOwnerContext& c) {
    construct_native_input_backend_base_00a908a0(owner, c);
    write<std::uint32_t>(owner, 0, 0x00d5b5f8);
    write<std::uint8_t>(owner, 0x64, 0);
    // All three A914C0 bodies are nonthrowing raw stores. The native iterator
    // constructs forward; no blanket initialization of their leading words.
    for (std::size_t index = 0; index != 3; ++index)
        construct_native_input_backend_class_00a914c0(at(owner, 0x68 + index * 0x24));
    write<std::uint8_t>(owner, 0xd4, 0); write<std::uint32_t>(owner, 0xd8, 0);
    write<std::uint8_t>(owner, 0xdc, 0); write<std::uint8_t>(owner, 0xdd, 0);
    for (std::size_t group = 0; group != 3; ++group) {
        for (std::size_t slot = 0; slot != 8; ++slot)
            write<std::uint32_t>(owner, 4 + group * 0x20 + slot * 4, 0);
        write<std::uint32_t>(owner, 0x68 + group * 0x24, 0);
    }
    return owner;
}
void destroy_native_input_backend_groups_00a90e00(void* owner,
    NativeInputBackendOwnerContext& c) {
    require_raw_lifetime(c);
    write<std::uint32_t>(owner, 0, 0x00d5b5f8);
    try {
        for (std::size_t slot = 0; slot != 24; ++slot) {
            void* const cell = at(owner, 4 + slot * 4);
            if (void* const current = read<void*>(cell, 0))
                c.host.reset_device_vslot14(current);
            // A90E53 reloads after the reset callback, which may replace it.
            if (void* const captured = read<void*>(cell, 0)) {
                if (InterlockedDecrement(static_cast<volatile LONG*>(at(captured, 4))) == 0)
                    c.host.zero_references_device_vslot00(captured);
                write<void*>(cell, 0, nullptr); // overwrite callback replacement too
            }
        }
    } catch (...) {
        // Do NOT continue unvisited devices after a callback throws. Native
        // state1 unwinds class buffers then base publication/registration.
        unwind_groups(owner, c);
        throw;
    }
    destroy_classes(owner);
    destroy_native_input_backend_base_00a90940(owner, c);
}
void* scalar_delete_native_input_backend_groups_00a91150(void* owner,
    std::uint8_t flags, NativeInputBackendOwnerContext& c) {
    destroy_native_input_backend_groups_00a90e00(owner, c);
    if ((flags & 1u) != 0) singleton_lifetime_free(owner);
    return owner;
}

void destroy_native_input_backend_guids_00a97b00(void* vector) noexcept {
    if (void* const allocation = read<void*>(vector, 4)) singleton_lifetime_free(allocation);
    write<std::uint32_t>(vector, 4, 0); write<std::uint32_t>(vector, 8, 0);
    write<std::uint32_t>(vector, 0xc, 0);
}
void* construct_native_input_backend_00a982d0(void* owner,
    NativeInputBackendOwnerContext& c) {
    construct_native_input_backend_groups_00a91570(owner, c);
    write<std::uint32_t>(owner, 0, 0x00d5b72c);
    write<void*>(owner, 0xe0, nullptr);
    write<std::uint32_t>(owner, 0xe8, 0); write<std::uint32_t>(owner, 0xec, 0);
    write<std::uint32_t>(owner, 0xf0, 0);
    try {
        void* const module = GetModuleHandleA(nullptr);
        write<std::uint8_t>(owner, 0xf4, 0);
        c.direct_input.create_interface_00c2e016(module, static_cast<void**>(at(owner, 0xe0)));
        c.direct_input.add_ref_vslot04(read<void*>(owner, 0xe0));
        // Native reloads +E0 after AddRef before its actual EnumDevices call.
        c.host.enumerate_devices_vslot10(read<void*>(owner, 0xe0), 0, owner, 1);
        for (std::int32_t index = 0; index != 4; ++index) {
            void* const allocation = allocate_object(0x240);
            void* device = nullptr;
            try {
                if (allocation) device = c.host.construct_xinput_device_00a9a5a0(allocation, index);
            } catch (...) {
                // DECE7C state2 / CB6A36 frees only the current raw allocation.
                singleton_lifetime_free(allocation);
                throw;
            }
            // State1 is restored before attachment. Failure here does not
            // acquire ownership of an unattached device for extra cleanup.
            c.host.attach_device_00a904e0(owner, -1, device);
        }
    } catch (...) {
        unwind_backend(owner, c);
        throw;
    }
    return owner;
}
void* scalar_delete_native_input_backend_00a97c00(void* owner,
    std::uint8_t flags, NativeInputBackendOwnerContext& c) {
    require_raw_lifetime(c);
    write<std::uint32_t>(owner, 0, 0x00d5b72c);
    destroy_native_input_backend_guids_00a97b00(at(owner, 0xe4));
    destroy_native_input_backend_groups_00a90e00(owner, c);
    if ((flags & 1u) != 0) singleton_lifetime_free(owner);
    return owner;
}
void* create_native_input_backend(NativeInputBackendOwnerContext& c) {
    void* const allocation = allocate_object(sizeof(NativeInputBackendStorage));
    try { return construct_native_input_backend_00a982d0(allocation, c); }
    catch (...) { singleton_lifetime_free(allocation); throw; }
}
} // namespace bsp
