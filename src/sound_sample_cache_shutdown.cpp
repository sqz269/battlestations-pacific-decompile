#include "bsp/sound_sample_cache_shutdown.hpp"

#include <iterator>

namespace bsp {
namespace {

struct AuxiliaryBaseCleanupOnUnwind {
    SoundAuxiliaryTreeOwner& owner;
    SoundOwnerLifetimeBindings& lifetime;
    bool armed = true;
    ~AuxiliaryBaseCleanupOnUnwind() noexcept {
        // DEC42C state0 -> -1, CB6280 -> A88180. As for native destructor
        // unwinding, another exception here cannot be recovered locally.
        if (armed) unregister_sound_auxiliary_tree_owner_00a88180(owner, lifetime);
    }
};

struct AuxiliaryTreeStorageCleanup {
    SoundAuxiliaryTreeOwner& owner;
    NativeStringStorage& strings;
    ~AuxiliaryTreeStorageCleanup() noexcept {
        // A885E0 on unwind / inline normal A8870B: A88490(begin,end) uses
        // A88400 to release keys in right/root/left (reverse inorder) order.
        // Keep the explicit count until the whole range has been destroyed.
        // std::map supplies node topology/allocation, not native STL layout.
        auto& tree = *owner.tree_04;
        while (!tree.empty()) {
            auto node = tree.extract(std::prev(tree.end()));
            destroy_native_string_header_0041dd20(&node.key(), strings);
        }
        owner.count_0c = 0; // A884E8, before head storage is freed.
        owner.tree_04.reset();
        owner.count_0c = 0; // A88725 / A8860C, after head pointer is cleared.
    }
};

} // namespace

void destroy_sound_sample_cache_00a886c0(SoundAuxiliaryTreeOwner& owner,
    SoundSampleCacheContext& cache, SoundOwnerLifetimeBindings& lifetime) {
    owner.native_vtable_00 = 0x00d5b460u;
    AuxiliaryBaseCleanupOnUnwind base{owner, lifetime};
    {
        // DEC434 state1 -> 0 destroys owner+4 before the singleton base.
        AuxiliaryTreeStorageCleanup tree{owner, cache.strings};
        clear_sound_sample_cache_00a82c00(owner, cache);
    }
    base.armed = false; // A8872C stores EH state=-1 BEFORE the base call.
    unregister_sound_auxiliary_tree_owner_00a88180(owner, lifetime);
}

SoundAuxiliaryTreeOwner* scalar_delete_sound_sample_cache_00a88750(
    SoundAuxiliaryTreeOwner* owner, std::uint32_t flags,
    SoundSampleCacheContext& cache, SoundOwnerLifetimeBindings& lifetime) {
    destroy_sound_sample_cache_00a886c0(*owner, cache, lifetime);
    if (flags & 1u) delete owner;
    return owner;
}

} // namespace bsp
