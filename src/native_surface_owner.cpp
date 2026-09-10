#include "bsp/native_surface_owner.hpp"

#include <cstring>
#include <new>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native surface owner reconstruction requires MSVC Win32.
#endif

namespace bsp {
namespace {
constexpr std::uint32_t surface_profile = 0x00d619a0u;
constexpr std::uint32_t refcounted_profile = 0x00ceb130u;

void release_string_preserving_fields(NativeString& name, SizedStoragePool& pool) noexcept {
    // 0041DD20 and the inlined owner path return the buffer without clearing
    // either string field; NativeString::release_to deliberately differs here.
    if (char* const data = name.data()) {
        const auto bytes = name.length() + 1u;
        pool.release_00bd1510(data, bytes);
    }
}

class DiagnosticStringGuard final {
public:
    explicit DiagnosticStringGuard(SizedStoragePool& pool) noexcept : pool_(pool) {}
    ~DiagnosticStringGuard() { release_string_preserving_fields(value, pool_); }
    NativeString value;
    DiagnosticStringGuard(const DiagnosticStringGuard&) = delete;
    DiagnosticStringGuard& operator=(const DiagnosticStringGuard&) = delete;
private:
    SizedStoragePool& pool_;
};

void visit_resource_support(NativeSurfaceOwnerContext& context) {
    (void)resource_support_singleton_00b3e730(context.actual_resource_support_0108fedc,
        context.actual_lifetime_01090aa0);
}
} // namespace

NativeSurfaceOwnerStorage* construct_native_surface_00b3f630(
    void* actual_pool_slot, IDirect3DSurface9* const surface, DWORD flags,
    std::uint8_t kind, NativeSurfaceOwnerContext& context) {
    const auto captured_scalar_bits = context.actual_one_00d7a24c;
    auto* owner = ::new (actual_pool_slot) NativeSurfaceOwnerStorage;
    owner->vtable_00 = refcounted_profile;
    owner->references_04.store(1, std::memory_order_relaxed);
    owner->scalar_bits_08 = captured_scalar_bits; // One native MOVSS load.
    owner->scalar_bits_0c = captured_scalar_bits;
    owner->vtable_00 = surface_profile;
    // Placement construction initializes exactly the NativeString's two words.
    owner->surface_2c = nullptr;
    owner->kind_30 = kind;
    owner->flags_28 = flags;
    try {
        if (surface) {
            surface->AddRef();
            surface->Release(); // Both calls use the original captured input.
        }
        bind_native_surface_00b3cc80(*owner, surface);
        if (surface) {
            surface->AddRef();
            surface->Release();
        }
        {
            PooledStringStorage storage(context.actual_string_pool_00419cc0);
            DiagnosticStringGuard temporary(context.actual_string_pool_00419cc0);
            temporary.value.resize_0041dd40(storage, 7, true);
            if (temporary.value.data()) {
                std::memcpy(temporary.value.data(), "Surface", temporary.value.length() + 1u);
            }
            // The native 12-byte stack diagnostic record has a borrowed COM
            // word followed by a second deep string. Neither retains the COM.
            IDirect3DSurface9* const diagnostic_borrowed_surface = surface;
            (void)diagnostic_borrowed_surface;
            DiagnosticStringGuard record_name(context.actual_string_pool_00419cc0);
            record_name.value.copy_from_00be0a30_fragment(storage, temporary.value);
            visit_resource_support(context);
            // Record string returns first, then the original temporary string.
        }
        const auto current_flags = owner->flags_28;
        if ((current_flags & 0x110u) != 0) ++context.actual_tracking_counter_0108dafc;
    } catch (...) {
        // Ctor states 1 -> 0: name cleanup then the refcounted base. There is no
        // COM cleanup or pool return in this constructor's exception map.
        release_string_preserving_fields(owner->name_10, context.actual_string_pool_00419cc0);
        owner->vtable_00 = refcounted_profile;
        throw;
    }
    return owner;
}

void bind_native_surface_00b3cc80(NativeSurfaceOwnerStorage& owner, IDirect3DSurface9* surface) {
    owner.surface_2c = surface;
    if (surface) {
        surface->AddRef();
        D3DSURFACE_DESC description; // Native stack is not zero initialized.
        (void)owner.surface_2c->GetDesc(&description);
        owner.format_18 = description.Format;
        owner.width_1c = description.Width;
        owner.height_20 = description.Height;
        owner.multisample_24 = description.MultiSampleType;
    } else {
        owner.format_18 = D3DFMT_UNKNOWN;
        owner.width_1c = 0;
        owner.height_20 = 0;
        owner.flags_28 = 0;
        owner.multisample_24 = D3DMULTISAMPLE_NONE;
    }
}

void destroy_native_surface_00b3f4e0(
    NativeSurfaceOwnerStorage& owner, NativeSurfaceOwnerContext& context) {
    owner.vtable_00 = surface_profile;
    try {
        auto* const captured_renderer = context.actual_renderer_00f8d394;
        (void)unregister_native_surface_00b27d60(*captured_renderer, &owner);
        visit_resource_support(context);
        if (auto* const current_surface = owner.surface_2c) {
            current_surface->Release();
            owner.surface_2c = nullptr; // After the callback, even if it changed +2C.
        }
        const auto current_flags = owner.flags_28;
        if ((current_flags & 0x110u) != 0) --context.actual_tracking_counter_0108dafc;
    } catch (...) {
        // Dtor state 1 releases the current name then state 0 installs the base.
        release_string_preserving_fields(owner.name_10, context.actual_string_pool_00419cc0);
        owner.vtable_00 = refcounted_profile;
        throw;
    }
    release_string_preserving_fields(owner.name_10, context.actual_string_pool_00419cc0);
    owner.vtable_00 = refcounted_profile; // 00BD30F0's complete base destructor.
}

NativeSurfaceOwnerStorage* delete_native_surface_00b3f5b0(
    NativeSurfaceOwnerStorage& owner, std::uint32_t flags, NativeSurfaceOwnerContext& context) {
    auto* const original_address = &owner;
    destroy_native_surface_00b3f4e0(owner, context);
    if ((flags & 1u) != 0) {
        owner.~NativeSurfaceOwnerStorage();
        context.actual_surface_pool_0108db00.return_raw_slot_00b3d860(original_address);
    }
    return original_address;
}

void release_native_surface_for_reset_00b3d510(NativeSurfaceOwnerStorage& owner) {
    if (auto* const captured_surface = owner.surface_2c) {
        captured_surface->AddRef();
        captured_surface->Release();
    }
    if (auto* const current_surface = owner.surface_2c) {
        current_surface->Release();
        owner.surface_2c = nullptr;
    }
}

HRESULT recreate_native_surface_00b3d550(NativeSurfaceOwnerStorage& owner, IDirect3DDevice9& device) {
    if (owner.kind_30 != 0) {
        return device.CreateDepthStencilSurface(owner.width_1c, owner.height_20, owner.format_18,
            owner.multisample_24, 0, TRUE, &owner.surface_2c, nullptr);
    }
    return device.CreateRenderTarget(owner.width_1c, owner.height_20, owner.format_18,
        owner.multisample_24, 0, FALSE, &owner.surface_2c, nullptr);
}

bool remove_first_native_surface_00b25630(
    NativeSurfacePointerArray& array, NativeSurfaceOwnerStorage* const* target) {
    // Native comparisons use unsigned address arithmetic, including the initial
    // empty-range check before reading the target pointer's storage.
    const auto first = reinterpret_cast<std::uintptr_t>(array.data_00);
    const auto captured_count = array.count_04;
    const auto end = first + captured_count * 4u;
    if (first >= end) return false;
    auto* const captured_target = *target;
    auto cursor = first;
    while (*reinterpret_cast<NativeSurfaceOwnerStorage**>(cursor) != captured_target) {
        cursor += 4u;
        if (cursor >= end) return false;
    }
    const auto index = static_cast<std::int32_t>(cursor - first) >> 2;
    if (index == -1) return false;
    if (static_cast<std::uint32_t>(index) != captured_count - 1u) {
        auto** const captured_data = reinterpret_cast<NativeSurfaceOwnerStorage**>(first);
        captured_data[index] = captured_data[captured_count - 1u];
    }
    --array.count_04; // Capacity and stale final entry are not cleared.
    return true;
}

bool unregister_native_surface_00b27d60(
    NativeSurfaceRendererStorage& actual_renderer, NativeSurfaceOwnerStorage* target) {
    return remove_first_native_surface_00b25630(actual_renderer.surfaces_1b0c, &target);
}

} // namespace bsp
