#include "bsp/native_render_service_texture_lifetime.hpp"
#include "bsp/native_ref_counted.hpp"
#include "bsp/native_render_service_texture_vectors.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <exception>
#include <new>
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native render-service texture lifetime requires MSVC Win32.
#endif

namespace bsp {
namespace {
static_assert(sizeof(void*) == 4 && sizeof(long) == 4);
static_assert(sizeof(std::atomic<std::int32_t>) == 4);
static_assert(alignof(std::atomic<std::int32_t>) == 4);
void* at(void* owner, std::uint32_t offset) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<std::uintptr_t>(owner) + offset);
}
std::uint32_t word(const void* location) noexcept {
    return *static_cast<const volatile std::uint32_t*>(location);
}
void put(void* location, std::uint32_t value) noexcept {
    *static_cast<volatile std::uint32_t*>(location) = value;
}
void* pointer(std::uint32_t value) noexcept { return reinterpret_cast<void*>(value); }
std::atomic<std::int32_t>& actual_count(void* owner) noexcept {
    return *std::launder(reinterpret_cast<std::atomic<std::int32_t>*>(at(owner, 4)));
}
void require_current_terminal(void* owner, std::uint32_t profile,
    const volatile std::uint32_t* table, std::uint32_t deleting) {
    // Caller slot0 fetch follows the zero result. BD30E0 reloads the current
    // profile before fetching slot4 and forwarding flags1; it never decrements.
    if (word(owner) != profile || !table || table[0] != 0x00bd30e0u)
        throw std::invalid_argument("unsupported current child slot0 profile");
    if (word(owner) != profile || table[1] != deleting)
        throw std::invalid_argument("unsupported refreshed child deleting slot");
}
void release_captured(void* owner, std::uint32_t offset, void* captured,
    NativeRenderServiceTextureDecrement decrement, bool auxiliary,
    NativeRenderServiceTextureLifetimeContext& context) {
    if (!captured) return;
    if (!decrement) throw std::invalid_argument("unbound current CE2220 IAT target");
    if (decrement(static_cast<volatile long*>(at(captured, 4))) == 0) {
        require_current_terminal(captured, auxiliary ? 0x00d61ec8u : 0x00d61948u,
            auxiliary ? context.actual_profile_00d61ec8 : context.actual_profile_00d61948,
            auxiliary ? 0x00b4e450u : 0x00b3f590u);
        auto& reference = context.actual_owners.resolve_actual(captured);
        if (&reference.reference_count != &actual_count(captured))
            throw std::invalid_argument("child companion must borrow captured actual+04");
        reference.release_zero_references(); // Canonical terminal; NO second decrement.
        // Captured native storage and the reference may no longer exist here.
    }
    put(at(owner, offset), 0); // Deliberately overwrite returning callback writes.
}
void release_current(void* owner, std::uint32_t offset,
    NativeRenderServiceTextureDecrement decrement, bool auxiliary,
    NativeRenderServiceTextureLifetimeContext& context) {
    release_captured(owner, offset, pointer(word(at(owner, offset))),
        decrement, auxiliary, context);
}
void destroy_vector(void* owner, std::uint32_t offset, bool stride24) {
    void* const header = at(owner, offset);
    if (stride24) resize_native_texture_vector24_00b52170(header, 0);
    else resize_native_texture_vector12_00b521e0(header, 0);
    singleton_lifetime_free(pointer(word(header)));
}
struct Cleanup {
    void* owner;
    int state{4};
    ~Cleanup() noexcept {
        try {
            while (state >= 0) {
                switch (state--) {
                case 4: destroy_native_texture_vector24_00b523c0(at(owner, 0xa4)); break;
                case 3: destroy_native_texture_vector12_00b523e0(at(owner, 0x8c)); break;
                case 2: destroy_native_texture_vector24_00b523c0(at(owner, 0x7c)); break;
                case 1: destroy_native_texture_vector24_00b523c0(at(owner, 0x24)); break;
                case 0: destroy_native_ref_counted_base_00bd30f0(owner); break;
                }
            }
        } catch (...) { std::terminate(); }
    }
};
std::atomic<std::int32_t>& binding_count(void* owner,
    NativeRenderServiceTextureLifetimeContext& context,
    const GuiNativeGeometryRegistration& registration) {
    if (!owner || reinterpret_cast<std::uintptr_t>(owner) % 4 ||
        &registration.owners != &context.actual_owners || !registration.find ||
        !registration.bind || !registration.unbind)
        throw std::invalid_argument("child companion requires aligned storage and same canonical registry");
    if (registration.find(registration.context, owner))
        throw std::invalid_argument("child already has a canonical companion");
    require_current_terminal(owner, 0x00d62074u, context.actual_profile_00d62074, 0x00b52840u);
    auto& count = actual_count(owner);
    if (count.load(std::memory_order_relaxed) <= 0)
        throw std::invalid_argument("child companion requires a live actual count");
    return count;
}
} // namespace

void release_native_render_service_texture_auxiliaries_00b52270(void* owner,
    NativeRenderServiceTextureLifetimeContext& context) {
    const auto decrement = context.decrement_iat_00ce2220; // B52271, one epoch.
    release_current(owner, 0x30, decrement, true, context);
    release_current(owner, 0x68, decrement, true, context);
    release_current(owner, 0x88, decrement, true, context);
    release_current(owner, 0xb0, decrement, true, context);
}

void destroy_native_render_service_textures_00b52400(void* owner,
    NativeRenderServiceTextureLifetimeContext& context) {
    put(owner, 0x00d62074u);
    Cleanup cleanup{owner};
    release_native_render_service_texture_auxiliaries_00b52270(owner, context);
    void* const first = pointer(word(at(owner, 0x18))); // B52433 before IAT.
    const auto decrement = context.decrement_iat_00ce2220; // B52436, fresh epoch.
    release_captured(owner, 0x18, first, decrement, false, context);
    release_current(owner, 0x3c, decrement, false, context);
    release_current(owner, 0x70, decrement, false, context);
    release_current(owner, 0x98, decrement, false, context);
    cleanup.state = 3; destroy_vector(owner, 0xa4, true);
    cleanup.state = 2; destroy_vector(owner, 0x8c, false);
    cleanup.state = 1; destroy_vector(owner, 0x7c, true);
    cleanup.state = 0; destroy_vector(owner, 0x24, true);
    cleanup.state = -1;
    destroy_native_ref_counted_base_00bd30f0(owner);
}

void* delete_native_render_service_textures_00b52840(void* owner,
    std::uint32_t flags, NativeRenderServiceTextureLifetimeContext& context) {
    destroy_native_render_service_textures_00b52400(owner, context);
    if (flags & 1) singleton_lifetime_free(owner);
    return owner;
}

NativeRenderServiceTextureReference::NativeRenderServiceTextureReference(void* owner,
    NativeRenderServiceTextureLifetimeContext& context,
    const GuiNativeGeometryRegistration& registration)
    : RenderCommandReference(binding_count(owner, context, registration)), identity_(owner),
      context_(context), registration_(registration) {
    registration_.bind(registration_.context, identity_, *this);
}
NativeRenderServiceTextureReference::~NativeRenderServiceTextureReference() {
    if (phase_ != Phase::retired) std::terminate();
}
bool NativeRenderServiceTextureReference::retired() const noexcept {
    return phase_ == Phase::retired;
}
void NativeRenderServiceTextureReference::release_zero_references() noexcept {
    if (phase_ != Phase::bound || reference_count.load(std::memory_order_relaxed) != 0)
        std::terminate();
    try {
        require_current_terminal(identity_, 0x00d62074u,
            context_.actual_profile_00d62074, 0x00b52840u);
        phase_ = Phase::destroying;
        delete_native_render_service_textures_00b52840(identity_, 1, context_);
        phase_ = Phase::retired;
        registration_.unbind(registration_.context, identity_, *this);
    } catch (...) { std::terminate(); }
}
} // namespace bsp
