#include "bsp/native_directional_shadow_owner_lifetime.hpp"

#include "bsp/native_camera_reference.hpp"
#include "bsp/native_frame_target_owner.hpp"
#include "bsp/native_ref_counted.hpp"
#include "bsp/native_texture_2d_owner.hpp"
#include "bsp/singleton_lifetime.hpp"

#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native directional shadow owner lifetime requires MSVC Win32.
#endif

namespace bsp {
namespace {
static_assert(sizeof(void*) == 4 && sizeof(long) == 4);

void* address(const void* owner, std::uint32_t offset = 0) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<std::uintptr_t>(owner) + offset);
}
__forceinline std::uint32_t word(const void* storage) noexcept {
    std::uint32_t result;
    __asm { mov eax, storage }
    __asm { mov eax, dword ptr [eax] }
    __asm { mov result, eax }
    return result;
}
__forceinline void put(void* storage, std::uint32_t value) noexcept {
    __asm { mov eax, storage }
    __asm { mov edx, value }
    __asm { mov dword ptr [eax], edx }
}
__forceinline std::uint8_t byte(const volatile std::uint32_t& storage) noexcept {
    const volatile std::uint32_t* location = &storage;
    std::uint8_t result;
    __asm { mov eax, location }
    __asm { mov al, byte ptr [eax] }
    __asm { mov result, al }
    return result;
}
void* pointer(std::uint32_t value) noexcept { return reinterpret_cast<void*>(value); }

enum class Terminal { shadow, viewport, texture_2d, frame_target };

const volatile std::uint32_t* table(std::uint32_t current_profile,
    Terminal domain, NativeDirectionalShadowOwnerContext& context) {
    const volatile std::uint32_t* result = nullptr;
    switch (domain) {
    case Terminal::shadow:
        if (current_profile == 0x00d5b5d8) result = context.table_00d5b5d8;
        break;
    case Terminal::viewport:
        if (current_profile == 0x00d5e5f8) result = context.table_00d5e5f8;
        break;
    case Terminal::texture_2d:
        if (current_profile == 0x00d61948) result = context.table_00d61948;
        break;
    case Terminal::frame_target:
        if (current_profile == 0x00d5e600) result = context.table_00d5e600;
        break;
    }
    if (!result) throw std::invalid_argument("Unsupported current shadow lifetime profile/table binding");
    return result;
}

class ConcreteDelete final : public NativeRefCountedDeleteCalls {
public:
    ConcreteDelete(Terminal domain, NativeDirectionalShadowOwnerContext& context)
        : domain_(domain), context_(context) {}
    void delete_vslot04(void* owner, std::uint32_t current_profile,
        std::uint32_t flags) override {
        const auto target = table(current_profile, domain_, context_)[1];
        switch (domain_) {
        case Terminal::shadow:
            if (target != 0x00a8fcd0) break;
            (void)delete_native_directional_shadow_owner_00a8fcd0(owner, flags, context_);
            return;
        case Terminal::viewport:
            if (target != 0x00b1f8f0) break;
            (void)delete_native_viewport_owner_00b1f8f0(
                static_cast<NativeViewportOwner*>(owner), flags);
            return;
        case Terminal::texture_2d:
            if (target != 0x00b3f590) break;
            (void)delete_native_texture_2d_00b3f590(owner, flags, context_.texture_2d);
            return;
        case Terminal::frame_target:
            if (target != 0x00b1fcf0) break;
            (void)delete_native_frame_target_owner_00b1fcf0(
                *static_cast<NativeFrameTargetOwnerStorage*>(owner), flags, context_.frame_target);
            return;
        }
        throw std::invalid_argument("Unsupported current shadow lifetime deleting slot");
    }
private:
    Terminal domain_;
    NativeDirectionalShadowOwnerContext& context_;
};

void release_zero(void* captured, Terminal domain,
    NativeDirectionalShadowOwnerContext& context) {
    const auto current_profile = word(captured);
    const auto slot0 = table(current_profile, domain, context)[0];
    if (slot0 != 0x00bd30e0)
        throw std::invalid_argument("Unsupported current shadow lifetime zero slot");
    ConcreteDelete calls(domain, context);
    // The real BD30E0 provider reloads the current owner profile; our concrete
    // slot4 dispatch reads that table anew. Neither step decrements again.
    invoke_native_ref_counted_delete_00bd30e0(captured, calls);
}

void release_captured_field(void* owner, std::uint32_t offset, void* captured,
    NativeDirectionalShadowInterlocked decrement, Terminal domain,
    NativeDirectionalShadowOwnerContext& context) {
    if (!captured) return;
    if (decrement(static_cast<volatile long*>(address(captured, 4))) == 0)
        release_zero(captured, domain, context);
    put(address(owner, offset), 0);
}
void release_current_group(void* owner, std::uint32_t offset,
    NativeDirectionalShadowOwnerContext& context) {
    auto* const captured = pointer(word(address(owner, offset)));
    if (!captured) return; // Native skips the IAT load as well as the call.
    const auto decrement = context.decrement_iat_00ce2220;
    release_captured_field(owner, offset, captured, decrement, Terminal::frame_target, context);
}

NativeCameraReference& camera(void* actual, NativeDirectionalShadowOwnerContext& context) {
    auto* const canonical = context.nodes.attachments.find_actual_node(
        reinterpret_cast<std::uint32_t>(actual));
    auto* const reference = dynamic_cast<NativeCameraReference*>(canonical);
    if (!reference || &reference->camera_owner().storage.node != actual)
        throw std::invalid_argument("Shadow lifetime needs the canonical actual camera companion");
    return *reference;
}
void release_camera_field(void* owner, std::uint32_t offset,
    NativeDirectionalShadowOwnerContext& context) {
    auto* const first = pointer(word(address(owner, offset)));
    // Unconditional native B71990. Missing/null identity is a source binding
    // diagnostic, never a successful skipped viewport operation.
    set_native_camera_viewport_00b71990(camera(first, context).camera_owner(), nullptr);
    if (auto* const reloaded = pointer(word(address(owner, offset)))) {
        unlink_and_release_render_model_00b6dfa0(camera(reloaded, context));
        put(address(owner, offset), 0);
    }
}

struct BaseCleanup {
    void* actual_owner;
    bool armed{true};
    ~BaseCleanup() noexcept {
        if (armed) unwind_native_directional_shadow_base_00cb63d0(actual_owner);
    }
};
} // namespace

void __fastcall unwind_native_directional_shadow_base_00cb63d0(void* owner) noexcept {
    destroy_native_ref_counted_base_00bd30f0(owner);
}

void destroy_native_directional_shadow_owner_00a8dec0(void* owner,
    NativeDirectionalShadowOwnerContext& context) {
    put(owner, 0x00d5b574);
    auto* const captured_504 = pointer(word(address(owner, 0x504)));
    const auto captured_decrement = context.decrement_iat_00ce2220;
    BaseCleanup cleanup{owner}; // Native state0 is armed only after these reads.
    release_captured_field(owner, 0x504, captured_504,
        captured_decrement, Terminal::viewport, context);
    for (std::uint32_t offset = 0x10; offset != 0x20; offset += 4)
        release_captured_field(owner, offset, pointer(word(address(owner, offset))),
            captured_decrement, Terminal::viewport, context);
    for (std::uint32_t offset = 0x20; offset != 0x30; offset += 4)
        release_camera_field(owner, offset, context);
    release_captured_field(owner, 0x384, pointer(word(address(owner, 0x384))),
        captured_decrement, Terminal::texture_2d, context);
    for (std::uint32_t offset = 0x4f0; offset != 0x500; offset += 4)
        release_current_group(owner, offset, context);
    release_current_group(owner, 0x500, context);
    cleanup.armed = false;
    destroy_native_ref_counted_base_00bd30f0(owner);
}

void* delete_native_directional_shadow_owner_00a8fcd0(void* owner,
    const volatile std::uint32_t& actual_public_flags_slot,
    NativeDirectionalShadowOwnerContext& context) {
    auto* const original = owner;
    destroy_native_directional_shadow_owner_00a8dec0(original, context);
    if ((byte(actual_public_flags_slot) & 1u) != 0) singleton_lifetime_free(original);
    return original;
}

void set_native_directional_light_shadow_owner_00b7bdf0(void* light,
    void* incoming, NativeDirectionalShadowOwnerContext& context) {
    auto* const captured_old = pointer(word(address(light, 0x174)));
    if (captured_old == incoming) return;
    put(address(light, 0x174), reinterpret_cast<std::uint32_t>(incoming));
    if (incoming) {
        const auto increment = context.increment_iat_00ce221c;
        (void)increment(static_cast<volatile long*>(address(incoming, 4)));
    }
    if (captured_old) {
        const auto decrement = context.decrement_iat_00ce2220;
        if (decrement(static_cast<volatile long*>(address(captured_old, 4))) == 0)
            release_zero(captured_old, Terminal::shadow, context);
    }
}
} // namespace bsp
