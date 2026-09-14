#include "bsp/native_viewport_owner.hpp"
#include "bsp/singleton_lifetime.hpp"

#include <array>
#include <cstring>
#include <new>
#include <stdexcept>
#include <type_traits>
#include <utility>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native viewport ownership requires MSVC Win32 pointer and LONG widths.
#endif

namespace bsp {
static_assert(sizeof(void*) == 4 && sizeof(long) == 4);
static_assert(std::is_standard_layout_v<NativeViewportOwner>);
static_assert(std::is_trivially_copyable_v<NativeViewportOwner>);
static_assert(sizeof(NativeViewportOwner) == kNativeViewportBytes);
static_assert(offsetof(NativeViewportOwner, references_04) == 0x04);
static_assert(offsetof(NativeViewportOwner, fields_08) == 0x08);
static_assert(sizeof(NativeViewportFields) == 0x2c);
static_assert(offsetof(NativeViewportFields, x) == 0x00);
static_assert(offsetof(NativeViewportFields, y) == 0x04);
static_assert(offsetof(NativeViewportFields, width) == 0x08);
static_assert(offsetof(NativeViewportFields, height) == 0x0c);
static_assert(offsetof(NativeViewportFields, depth_min_bits) == 0x10);
static_assert(offsetof(NativeViewportFields, depth_max_bits) == 0x14);
static_assert(offsetof(NativeViewportFields, scissor_enabled) == 0x18);
static_assert(offsetof(NativeViewportFields, preserved_21) == 0x19);
static_assert(offsetof(NativeViewportFields, scissor) == 0x1c);

NativeViewportView::NativeViewportView(NativeViewportOwner& value) noexcept
    : owner(value), x(value.fields_08.x), y(value.fields_08.y),
      width(value.fields_08.width), height(value.fields_08.height),
      scissor_enabled(value.fields_08.scissor_enabled), scissor(value.fields_08.scissor) {}

namespace {
NativeViewportRendererParameters parameters_from_live_renderer(
    NativeViewportEnvironment& environment) {
    auto* const captured = environment.renderer_00f8d394;
    if (!captured) throw std::invalid_argument("native viewport: global00F8D394 is unbound");
    return environment.renderer_access.parameters_00b1ff60(*captured);
}

void copy_two_words_forward(void* destination, const void* source) noexcept {
    __asm {
        mov ecx, destination
        mov eax, source
        mov edx, dword ptr [eax]
        mov dword ptr [ecx], edx
        mov eax, dword ptr [eax + 4]
        mov dword ptr [ecx + 4], eax
    }
}
} // namespace

NativeViewportOwner* initialize_native_viewport_owner_00b1f850(
    void* storage, NativeViewportEnvironment& environment) {
    if (!storage || (reinterpret_cast<std::uintptr_t>(storage) % alignof(NativeViewportOwner))) {
        throw std::invalid_argument("native viewport: missing or misaligned raw storage");
    }
    // Preserve raw representation before starting its C++ lifetime, including
    // byte20 seen by renderer callbacks BEFORE the last constructor store.
    std::array<unsigned char, kNativeViewportBytes> preimage;
    std::memcpy(preimage.data(), storage, preimage.size());
    auto* const owner = ::new (storage) NativeViewportOwner;
    std::memcpy(owner, preimage.data(), preimage.size());
    owner->native_vtable_00 = kNativeViewportBaseVtable;
    owner->references_04 = 1;
    owner->native_vtable_00 = kNativeViewportVtable;
    auto& fields = owner->fields_08;
    fields.depth_min_bits = 0;
    const auto captured_one = environment.one_bits_00d7a24c;
    fields.x = 0;
    fields.y = 0;
    fields.width = 640;
    fields.height = 480;
    fields.depth_max_bits = captured_one;
    try {
        fields.width = parameters_from_live_renderer(environment).width_0c;
        fields.height = parameters_from_live_renderer(environment).height_10;
        fields.scissor_enabled = 0;
    } catch (...) {
        owner->native_vtable_00 = kNativeViewportBaseVtable; // 00CBCCB0 -> 00BD30F0
        owner->~NativeViewportOwner();
        throw;
    }
    return owner;
}

NativeViewportOwner* allocate_native_viewport_owner(NativeViewportEnvironment& environment) {
    void* const storage = singleton_lifetime_allocate({SingletonAllocationKind::object,
        kNativeViewportBytes, sizeof(NativeViewportOwner)});
    try {
        return initialize_native_viewport_owner_00b1f850(storage, environment);
    } catch (...) {
        singleton_lifetime_free(storage);
        throw;
    }
}

NativeViewportOwner* allocate_native_viewport_owner(NativeViewportEnvironment& environment,
    NativeViewportRegistry::Admission&& admission) {
    auto& registry = admission.require_registry(); // before native allocation
    auto prepared = std::move(admission); // callbacks cannot consume the caller's token
    auto* const owner = allocate_native_viewport_owner(environment);
    registry.constructed(prepared, *owner);
    return owner;
}

void retain_native_viewport_owner(NativeViewportOwner& owner) noexcept {
    (void)::InterlockedIncrement(&owner.references_04);
}
void release_native_viewport_owner(NativeViewportOwner& owner) noexcept {
    if (::InterlockedDecrement(&owner.references_04) == 0) {
        invoke_native_viewport_deleting_destructor_00bd30e0(&owner);
    }
}
void invoke_native_viewport_deleting_destructor_00bd30e0(NativeViewportOwner* owner) noexcept {
    if (owner) (void)delete_native_viewport_owner_00b1f8f0(owner, 1);
}
NativeViewportOwner* delete_native_viewport_owner_00b1f8f0(
    NativeViewportOwner* owner, std::uint32_t flags) noexcept {
    auto* const original = owner;
    retire_registered_native_viewport_before_destroy(*owner);
    owner->native_vtable_00 = kNativeViewportVtable;
    owner->native_vtable_00 = kNativeViewportBaseVtable;
    owner->~NativeViewportOwner();
    if ((flags & 1u) != 0) singleton_lifetime_free(owner);
    return original; // includes native returning free tail at 00B1F90B
}

void set_native_viewport_origin_00b1f920(NativeViewportOwner& owner, const void* xy) noexcept {
    copy_two_words_forward(&owner.fields_08.x, xy);
}
void set_native_viewport_dimensions_00b1f940(NativeViewportOwner& owner, const void* wh) noexcept {
    copy_two_words_forward(&owner.fields_08.width, wh);
}
void set_native_viewport_min_depth_00b1f750(
    NativeViewportOwner& owner, std::uint32_t bits) noexcept {
    owner.fields_08.depth_min_bits = bits;
}
void set_native_viewport_max_depth_00b1f760(
    NativeViewportOwner& owner, std::uint32_t bits) noexcept {
    owner.fields_08.depth_max_bits = bits;
}
} // namespace bsp
