#include "bsp/native_light_lifetime.hpp"
#include "bsp/directional_light_pool.hpp"
#include "bsp/native_ref_counted.hpp"
#include "bsp/system_lighting_owners.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <atomic>
#include <cstddef>
#include <cstring>
#include <exception>
#include <new>
#include <stdexcept>
#include <type_traits>

namespace bsp {
namespace {
using Word = std::uint32_t;
static_assert(sizeof(void*) == 4);
void* at(void* p, Word offset) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<Word>(p) + offset);
}
Word read(const void* p, Word offset = 0) noexcept {
    return *reinterpret_cast<const volatile Word*>(reinterpret_cast<Word>(p) + offset);
}
void write(void* p, Word offset, Word value) noexcept {
    *static_cast<volatile Word*>(at(p, offset)) = value;
}
std::int32_t signed_word(Word value) noexcept {
    std::int32_t result;
    std::memcpy(&result, &value, sizeof(result));
    return result;
}
void* back(void* light) noexcept {
    const Word count = read(light, 0x17c);
    const Word begin = read(light, 0x178);
    return reinterpret_cast<void*>(read(reinterpret_cast<void*>(begin + count * 4u - 4u)));
}
RenderCommandReference& canonical(void* owner, NativeLightLifetimeContext& c) {
    auto& reference = c.node.scenes.owners.resolve_actual(owner);
    if (static_cast<void*>(&reference.reference_count) != at(owner, 4) ||
        reference.reference_count.load(std::memory_order_relaxed) != 0)
        throw std::logic_error("light terminal requires SAME actual+4 observed zero");
    return reference;
}
void require_context(NativeLightLifetimeContext& c) {
    if (&c.node.scenes.owners != &c.node.trees.owners ||
        &c.node.scenes.decrement_00ce2220 != &c.node.trees.decrement_00ce2220)
        throw std::invalid_argument("light requires SAME actual owner registry and decrement cell");
}
void require_slot(Word profile, NativeDirectionalLightIdentityContext& c, unsigned slot, Word target) {
    if (profile != 0x00d62fb0u || !c.actual_profile_00d62fb0 ||
        c.actual_profile_00d62fb0[slot] != target)
        throw std::logic_error("directional light lacks its genuine current native slot");
}
std::atomic<std::int32_t>& bind_count(void* owner, NativeDirectionalLightIdentityContext& c,
    NativeLightLifetimeAcquired& acquired) {
    require_context(c.lifetime);
    if (!owner || reinterpret_cast<Word>(owner) % 4 || acquired.started ||
        &c.owners != &c.lifetime.node.scenes.owners || c.owners.find(owner))
        throw std::invalid_argument("directional light binding requires unregistered completed actual storage");
    require_slot(read(owner), c, 0, 0x00bd30e0u);
    require_slot(read(owner), c, 1, 0x00b7c820u);
    auto& count = *std::launder(reinterpret_cast<std::atomic<std::int32_t>*>(at(owner, 4)));
    if (count.load(std::memory_order_relaxed) <= 0)
        throw std::invalid_argument("directional light binding requires live actual+4");
    return count;
}
class DirectionalDeleteCalls final : public NativeRefCountedDeleteCalls {
public:
    DirectionalDeleteCalls(void* owner, NativeDirectionalLightStorageReference& reference,
        NativeDirectionalLightIdentityContext& context)
        : owner_(owner), reference_(reference), context_(context) {}
    void delete_vslot04(void* owner, Word profile, Word flags) override {
        if (owner != owner_ || flags != 1)
            throw std::logic_error("directional terminal requires same owner and flags1");
        require_slot(profile, context_, 1, 0x00b7c820u);
        const volatile Word actual_flags = flags;
        reference_.delete_scalar_00b7c820(actual_flags);
    }
private:
    void* const owner_;
    NativeDirectionalLightStorageReference& reference_;
    NativeDirectionalLightIdentityContext& context_;
};
} // namespace

void destroy_native_light_scene_array_00b7c1c0(void* array) noexcept {
    // Exact12B pointer-only view; valid count0 resize never dereferences a
    // SceneResource object or grows. Native leaves begin/capacity stale.
    static_assert(sizeof(SystemAmbientBacklinks) == 12);
    static_assert(std::is_trivially_copyable_v<SystemAmbientBacklinks>);
    static_assert(std::is_trivially_destructible_v<SystemAmbientBacklinks>);
    // The raw Light constructor writes these words in assembly. Establish
    // this trivial descriptor's C++ lifetime and restore its complete bytes
    // before the first typed helper access; no callback or owning cleanup.
    std::byte preimage[12];
    std::memcpy(preimage, array, sizeof(preimage));
    auto* const links = ::new (array) SystemAmbientBacklinks;
    std::memcpy(links, preimage, sizeof(preimage));
    resize_system_ambient_backlinks_00b7bc70(*links, 0);
    void* const current_begin = reinterpret_cast<void*>(read(array));
    singleton_lifetime_free(current_begin);
}

void destroy_native_light_storage_00b7c5b0(void* light,
    NativeLightLifetimeFrame& frame, NativeLightLifetimeContext& context,
    NativeLightLifetimeAcquired& acquired) {
    require_context(context);
    if (acquired.started) throw std::invalid_argument("light destruction requires fresh persistent diagnostics");
    acquired.started = true;
    int state = -1;
    try {
        write(light, 0, 0x00d62f58);
        const Word initial_count = read(light, 0x17c); // B7C5D5
        const auto decrement = context.node.scenes.decrement_00ce2220; // B7C5DD, once
        acquired.captured_decrement = decrement;
        state = acquired.native_eh_state = 1;
        if (signed_word(initial_count) > 0) {
            do {
                void* const scene = back(light); // fresh count, then begin
                acquired.captured_unregister_scene = scene;
                frame.scene_remove.node_argument = reinterpret_cast<Word>(light);
                acquired.active_call_site = 0x00b7c601;
                unregister_native_scene_node_if_light_00b83ec0(scene, frame.scene_remove,
                    context.node.scenes.gates);
                void* const released = back(light); // CURRENT count/begin after gate
                acquired.captured_released_scene = released;
                acquired.active_call_site = 0x00b7c61a;
                if (decrement(static_cast<volatile long*>(at(released, 4))) == 0) {
                    acquired.active_call_site = 0x00b7c626;
                    canonical(released, context).release_zero_references();
                }
                ++acquired.completed_scene_releases;
                const Word count = read(light, 0x17c); // AFTER terminal callback
                if (count != 0) write(light, 0x17c, count - 1u);
            } while (signed_word(read(light, 0x17c)) > 0);
        }
        void* const shadow = reinterpret_cast<void*>(read(light, 0x174));
        acquired.captured_shadow = shadow;
        if (shadow) {
            acquired.active_call_site = 0x00b7c652;
            const long remaining = decrement(static_cast<volatile long*>(at(shadow, 4)));
            acquired.shadow_decrement_completed = true;
            if (remaining == 0) {
                acquired.active_call_site = 0x00b7c65e;
                const Word profile = read(shadow);
                if (!context.shadow_zero)
                    throw std::logic_error("zero shadow requires genuine family-specific terminal binding");
                const auto* const table = context.shadow_zero->resolve_profile(profile);
                if (!table) throw std::logic_error("shadow profile has no actual table binding");
                const Word target = table[0];
                auto& reference = canonical(shadow, context);
                context.shadow_zero->invoke_virtual0(target, shadow, reference);
                acquired.shadow_terminal_completed = true;
            }
            write(light, 0x174, 0); // AFTER callback, no later shadow read
        }
        state = acquired.native_eh_state = 0; // B7C674, before array cleanup
        acquired.active_call_site = 0x00b7c679;
        destroy_native_light_scene_array_00b7c1c0(at(light, 0x178));
        acquired.array_cleanup_completed = true;
        state = acquired.native_eh_state = -1; // B7C68B, before node call
        acquired.active_call_site = 0x00b7c693;
        destroy_native_raw_node_base_00b6f440(light, frame.node, context.node, acquired.node);
        acquired.complete = true;
    } catch (...) {
        acquired.exception_cleanup_started = true;
        try {
            while (state >= 0) {
                const int current = state--;
                acquired.native_eh_state = state;
                if (current == 1) {
                    destroy_native_light_scene_array_00b7c1c0(at(light, 0x178));
                    acquired.array_cleanup_completed = true;
                } else {
                    destroy_native_raw_node_base_00b6f440(light, frame.node, context.node, acquired.node);
                }
            }
        } catch (...) { std::terminate(); }
        throw;
    }
}

void* delete_native_directional_light_storage_00b7c820(void* light,
    const volatile Word& flags, DirectionalLightPool& pool,
    NativeLightLifetimeFrame& frame, NativeLightLifetimeContext& context,
    NativeLightLifetimeAcquired& acquired) {
    write(light, 0, 0x00d62fb0);
    destroy_native_light_storage_00b7c5b0(light, frame, context, acquired);
    if ((*reinterpret_cast<const volatile unsigned char*>(&flags) & 1u) != 0) {
        acquired.active_call_site = 0x00b7c83b;
        pool.return_raw_slot_00b7b2f0(light);
        acquired.pool_returned = true;
    }
    return light;
}

NativeDirectionalLightStorageReference::NativeDirectionalLightStorageReference(void* owner,
    NativeDirectionalLightIdentityContext& context, NativeLightLifetimeFrame& frame,
    NativeLightLifetimeAcquired& acquired)
    : RenderCommandReference(bind_count(owner, context, acquired)), identity_(owner),
      context_(context), frame_(frame), acquired_(acquired) {
    context_.owners.bind(owner, *this);
}
NativeDirectionalLightStorageReference::~NativeDirectionalLightStorageReference() {
    if (phase_ != Phase::retired) std::terminate();
}
bool NativeDirectionalLightStorageReference::retired() const noexcept { return phase_ == Phase::retired; }
void NativeDirectionalLightStorageReference::retire() noexcept {
    phase_ = Phase::retired;
    context_.owners.unbind(identity_, *this);
}
void* NativeDirectionalLightStorageReference::delete_scalar_00b7c820(const volatile Word& flags) {
    if (phase_ != Phase::bound)
        throw std::logic_error("directional scalar deletion requires its bound companion");
    phase_ = Phase::destroying;
    void* result;
    try { result = delete_native_directional_light_storage_00b7c820(identity_, flags,
        context_.pool_01090154, frame_, context_.lifetime, acquired_); }
    catch (...) { retire(); throw; }
    retire();
    return result;
}
void NativeDirectionalLightStorageReference::release_zero_references() noexcept {
    if (phase_ != Phase::bound || reference_count.load(std::memory_order_relaxed) != 0)
        std::terminate();
    try {
        require_slot(read(identity_), context_, 0, 0x00bd30e0u);
        DirectionalDeleteCalls calls(identity_, *this, context_);
        invoke_native_ref_counted_delete_00bd30e0(identity_, calls);
    } catch (...) { std::terminate(); }
}
} // namespace bsp
