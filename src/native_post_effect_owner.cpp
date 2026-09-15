#include "bsp/native_post_effect_owner.hpp"
#include "bsp/native_logical_vertex_owner.hpp"
#include "bsp/native_material_owner.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <exception>
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native post-effect ownership requires MSVC Win32.
#endif

namespace bsp {
namespace {
using Word = std::uint32_t;
static_assert(sizeof(void*) == 4 && sizeof(long) == 4 && sizeof(std::atomic<std::int32_t>) == 4);
static_assert(alignof(std::atomic<std::int32_t>) == 4);
Word word(const volatile void* base, Word byte_offset = 0) noexcept {
    Word value;
    __asm {
        mov eax, base
        mov edx, byte_offset
        mov eax, dword ptr [eax + edx]
        mov value, eax
    }
    return value;
}
void put(void* base, Word byte_offset, Word value) noexcept {
    __asm {
        mov eax, base
        mov edx, byte_offset
        mov ecx, value
        mov dword ptr [eax + edx], ecx
    }
}
void* pointer(Word bits) noexcept { return reinterpret_cast<void*>(bits); }
std::atomic<std::int32_t>& count(void* actual) noexcept {
    return *reinterpret_cast<std::atomic<std::int32_t>*>(reinterpret_cast<Word>(actual) + 4u);
}
void require(bool condition, const char* message) {
    if (!condition) throw std::logic_error(message);
}
void current_terminal(void* actual, Word profile, const volatile Word* table, Word deleting) {
    require(word(actual) == profile && table && word(table) == 0x00bd30e0u,
        "unsupported current post-effect member virtual-zero profile");
    // BD30E0 reloads the current profile before reading its deleting slot.
    require(word(actual) == profile && word(table, 4) == deleting,
        "unsupported current post-effect member deleting profile");
}
void release_registered(void* captured, bool stream, NativePostEffectDecrement decrement,
    NativePostEffectOwnerContext& context) {
    if (decrement(reinterpret_cast<volatile long*>(reinterpret_cast<Word>(captured) + 4u)) != 0)
        return;
    if (stream)
        current_terminal(captured, 0x00d61d6cu, context.actual_stream_profile_00d61d6c, 0x00b4bf10u);
    else
        current_terminal(captured, 0x00d5e520u, context.actual_material_profile_00d5e520, 0x00b194b0u);
    auto& reference = context.actual_owners.resolve_actual(captured);
    require(&reference.reference_count == &count(captured), "post-effect member has a foreign counter");
    if (stream) {
        auto* binding = dynamic_cast<NativeLogicalVertexReference*>(&reference);
        require(binding && binding->storage() == captured, "post-effect stream lacks its concrete companion");
        binding->release_zero_references();
    } else {
        auto* binding = dynamic_cast<NativeMaterialReference*>(&reference);
        require(binding && &binding->storage() == captured, "post-effect material lacks its concrete companion");
        binding->release_zero_references();
    }
    // The terminal may retire both native storage and its companion.
}
void release_node(Word captured, NativePostEffectOwnerContext& context) {
    auto* binding = context.nodes.attachments.find_actual_node(captured);
    require(binding != nullptr, "post-effect member has no canonical actual node binding");
    unlink_and_release_render_model_00b6dfa0(*binding);
}
}

NativePostEffectOwner::NativePostEffectOwner(void* actual, NativePostEffectOwnerContext& context)
    : actual_(actual), context_(context) {
    require(actual && (reinterpret_cast<Word>(actual) & 3u) == 0, "invalid actual post-effect storage");
    current_terminal(actual, 0x00d61ec8u, context.actual_profile_00d61ec8, 0x00b4e450u);
    require(count(actual).load(std::memory_order_relaxed) > 0, "post-effect binding requires a live count");
}

void destroy_native_post_effect_owner_00b4e2f0(NativePostEffectOwner& owner) {
    require(owner.phase_ == NativePostEffectOwner::Phase::live, "post-effect destruction cannot be retried");
    owner.phase_ = NativePostEffectOwner::Phase::destroying;
    void* const actual = owner.actual_;
    auto& context = owner.context_;
    put(actual, 0, 0x00d61ec8u); // B4E310
    const Word stream = word(actual, 0x18); // B4E316, before IAT capture
    const auto decrement = context.actual_decrement_00ce2220; // B4E319
    try { // Native state0 is armed at B4E323, after capture.
        if (stream) {
            release_registered(pointer(stream), true, decrement, context);
            put(actual, 0x18, 0); // B4E33B, after terminal return
        }
        if (const Word material = word(actual, 0x14)) {
            release_registered(pointer(material), false, decrement, context);
            put(actual, 0x14, 0); // B4E357
        }
        if (const Word node = word(actual, 0x0c)) {
            release_node(node, context); // B4E361
            put(actual, 0x0c, 0); // B4E366
        }
        if (const Word node = word(actual, 0x10)) {
            release_node(node, context); // B4E370
            put(actual, 0x10, 0); // B4E375
        }
        if (const Word allocation = word(actual, 0x1c)) {
            singleton_lifetime_free(pointer(allocation)); // B4E380
            put(actual, 0x1c, 0); // B4E388, recovered after returning free
        }
        if (const Word frame = word(actual, 0x08)) {
            if (decrement(reinterpret_cast<volatile long*>(frame + 4u)) == 0) {
                current_terminal(pointer(frame), 0x00d5e600u,
                    context.actual_frame_profile_00d5e600, 0x00b1fcf0u);
                delete_native_frame_target_owner_00b1fcf0(
                    *static_cast<NativeFrameTargetOwnerStorage*>(pointer(frame)), 1, context.frame_targets);
            }
            put(actual, 0x08, 0); // B4E3A4
        }
    } catch (...) {
        put(actual, 0, 0x00ceb130u); // CBFB10 -> BD30F0; no later-member retry/free
        owner.phase_ = NativePostEffectOwner::Phase::dead;
        throw;
    }
    put(actual, 0, 0x00ceb130u); // B4E3B1, state -1
    owner.phase_ = NativePostEffectOwner::Phase::dead;
}

void* delete_native_post_effect_owner_00b4e450(NativePostEffectOwner& owner, std::uint32_t flags) {
    void* const actual = owner.storage();
    destroy_native_post_effect_owner_00b4e2f0(owner); // B4E453
    if (flags & 1u) singleton_lifetime_free(actual); // B4E460, success only
    return actual;
}

NativePostEffectReference::NativePostEffectReference(NativePostEffectOwner& owner,
    GuiNativeGeometryRegistration registration, NativePostEffectCompanionDisposal disposal)
    : RenderCommandReference(count(owner.actual_)), owner_(owner),
      registration_(registration), disposal_(disposal) {
    require(owner.phase_ == NativePostEffectOwner::Phase::live && !owner.reference_bound_ &&
        &registration.owners == &owner.context_.actual_owners && registration.bind &&
        registration.unbind && registration.find && disposal.retire,
        "post-effect reference requires one live canonical registration and retirement");
    current_terminal(owner.actual_, 0x00d61ec8u, owner.context_.actual_profile_00d61ec8, 0x00b4e450u);
    require(reference_count.load(std::memory_order_relaxed) > 0 &&
        !registration.find(registration.context, owner.actual_), "duplicate or retired post-effect reference");
    registration.bind(registration.context, owner.actual_, *this); // transactional, no native effect
    owner.reference_bound_ = true;
}
NativePostEffectReference::~NativePostEffectReference() {
    if (!retired_) std::terminate();
}
void NativePostEffectReference::release_zero_references() noexcept {
    if (retired_ || owner_.phase_ != NativePostEffectOwner::Phase::live ||
        reference_count.load(std::memory_order_relaxed) != 0) std::terminate();
    current_terminal(owner_.actual_, 0x00d61ec8u, owner_.context_.actual_profile_00d61ec8, 0x00b4e450u);
    const auto registration = registration_;
    const auto disposal = disposal_;
    void* const actual = owner_.actual_;
    delete_native_post_effect_owner_00b4e450(owner_, 1);
    owner_.reference_bound_ = false; // host metadata only; native allocation is dead
    retired_ = true;
    registration.unbind(registration.context, actual, *this);
    disposal.retire(disposal.context, *this);
    // Host disposal may destroy both companions; no access follows it.
}
} // namespace bsp
