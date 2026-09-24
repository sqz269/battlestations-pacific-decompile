#include "bsp/native_group_storage_lifetime.hpp"
#include "bsp/gui_page_root.hpp"
#include "bsp/native_light_lifetime.hpp"
#include "bsp/native_ref_counted.hpp"
#include "bsp/system_lighting_owners.hpp"
#include <atomic>
#include <cstring>
#include <exception>
#include <new>
#include <stdexcept>
#include <type_traits>

namespace bsp {
namespace {
using Word = std::uint32_t;
void* at(void* p, Word offset) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<Word>(p) + offset);
}
Word read(const void* p, Word offset = 0) noexcept {
    return *reinterpret_cast<const volatile Word*>(reinterpret_cast<Word>(p) + offset);
}
void write(void* p, Word offset, Word value) noexcept {
    *static_cast<volatile Word*>(at(p, offset)) = value;
}
void require_context(const NativeNodeBaseDestructionContext& context) {
    if (&context.scenes.owners != &context.trees.owners ||
        &context.scenes.decrement_00ce2220 != &context.trees.decrement_00ce2220)
        throw std::invalid_argument("group lifetime requires SAME actual owners and decrement cell");
}
void require_live_pool_slot(void* owner, NativeGroupPool& pool) {
    if (!owner || reinterpret_cast<Word>(owner) % 4)
        throw std::invalid_argument("group companion requires actual aligned18Ch slot");
    const auto& storage = pool.storage();
    const Word index = NativeGroupPool::live_slab_index(owner);
    if (!storage.slabs_28 || index >= storage.slab_count_2c)
        throw std::invalid_argument("group companion requires SAME allocated pool slab");
    const auto* slab = storage.slabs_28[index];
    const Word offset = reinterpret_cast<Word>(owner) - reinterpret_cast<Word>(slab);
    if (!slab || offset >= NativeGroupPool::slot_bytes * NativeGroupPool::slots_per_slab ||
        offset % NativeGroupPool::slot_bytes != 0)
        throw std::invalid_argument("group companion requires actual slab slot identity");
    const Word count = *reinterpret_cast<const volatile std::uint16_t*>(slab + 0x31c0);
    if (count > NativeGroupPool::slots_per_slab)
        throw std::invalid_argument("invalid actual group slab free-count");
    const auto* free_slots = reinterpret_cast<const volatile std::uint16_t*>(slab + 0x3180);
    for (Word i = 0; i < count; ++i)
        if (free_slots[i] == offset / NativeGroupPool::slot_bytes)
            throw std::invalid_argument("group companion cannot admit returned slot");
}
void require_slot(Word profile, NativeGroupStorageIdentityContext& context,
    unsigned slot, Word target) {
    if (profile != 0x00d634f8 || !context.actual_profile_00d634f8 ||
        context.actual_profile_00d634f8[slot] != target)
        throw std::logic_error("group lacks its genuine current native slot");
}
std::atomic<std::int32_t>& bind_count(void* owner, NativeGroupStorageIdentityContext& c,
    NativeGroupStorageLifetimeAcquired& acquired) {
    require_live_pool_slot(owner, c.pool_010902f4);
    require_context(c.lifetime);
    if (acquired.started || acquired.node.started || acquired.descriptor_prepared ||
        &c.owners != &c.lifetime.scenes.owners || c.owners.find(owner))
        throw std::invalid_argument("group admission requires fresh unregistered completed storage");
    require_slot(read(owner), c, 0, 0x00bd30e0);
    require_slot(read(owner), c, 1, 0x00b8f8c0);
    auto& count = *std::launder(reinterpret_cast<std::atomic<std::int32_t>*>(at(owner, 4)));
    if (count.load(std::memory_order_relaxed) <= 0)
        throw std::invalid_argument("group admission requires live actual+4");
    return count;
}
void prepare_descriptor(void* owner) noexcept {
    static_assert(std::is_trivially_destructible_v<NativeGroupTailStorage>);
    static_assert(sizeof(SystemAmbientBacklinks) == 12);
    static_assert(std::is_trivially_copyable_v<SystemAmbientBacklinks>);
    std::byte preimage[12];
    std::memcpy(preimage, at(owner, 0x178), sizeof preimage);
    // Successful raw constructor's old actual tail view ends here. Its other
    // bytes remain raw preimages; no later code uses that obsolete typed view.
    std::launder(reinterpret_cast<NativeGroupTailStorage*>(at(owner, 0x174)))->~NativeGroupTailStorage();
    auto* array = ::new (at(owner, 0x178)) SystemAmbientBacklinks;
    std::memcpy(array, preimage, sizeof preimage);
}
class GroupDeleteCalls final : public NativeRefCountedDeleteCalls {
public:
    GroupDeleteCalls(void* owner, NativeGroupStorageReference& reference,
        NativeGroupStorageIdentityContext& context)
        : owner_(owner), reference_(reference), context_(context) {}
    void delete_vslot04(void* owner, Word profile, Word flags) override {
        if (owner != owner_ || flags != 1)
            throw std::logic_error("group terminal requires SAME owner and flags1");
        require_slot(profile, context_, 1, 0x00b8f8c0);
        const volatile Word actual_flags = flags;
        reference_.delete_scalar_00b8f8c0(actual_flags);
    }
private:
    void* const owner_;
    NativeGroupStorageReference& reference_;
    NativeGroupStorageIdentityContext& context_;
};
}

void destroy_native_group_storage_00b8f680(void* owner,
    NativeGroupStorageLifetimeFrame& frame, NativeNodeBaseDestructionContext& context,
    NativeGroupStorageLifetimeAcquired& acquired) {
    if (acquired.started || acquired.node.started)
        throw std::invalid_argument("group destruction requires fresh persistent diagnostics");
    require_context(context);
    acquired.started = true;
    int state = -1;
    try {
        write(owner, 0, 0x00d634f8);
        state = acquired.native_eh_state = 0;
        acquired.active_call_site = 0x00b8f6b6;
        // Proven pointer-only resize/current begin/free sequence; no entry A0
        // clearing and no point-light allocation registry or SceneResource view.
        destroy_native_light_scene_array_00b7c1c0(at(owner, 0x178));
        acquired.array_cleanup_completed = true;
        state = acquired.native_eh_state = -1; // B8F6C8 before node cleanup
        acquired.active_call_site = 0x00b8f6d0;
        destroy_native_raw_node_base_00b6f440(owner, frame.node, context, acquired.node);
        acquired.complete = true;
    } catch (...) {
        acquired.exception_cleanup_started = true;
        if (state == 0) {
            state = acquired.native_eh_state = -1;
            try { destroy_native_raw_node_base_00b6f440(owner, frame.node, context, acquired.node); }
            catch (...) { std::terminate(); }
        }
        throw;
    }
}

void* delete_native_group_storage_00b8f8c0(void* owner,
    const volatile Word& flags, NativeGroupPool& pool,
    NativeGroupStorageLifetimeFrame& frame, NativeNodeBaseDestructionContext& context,
    NativeGroupStorageLifetimeAcquired& acquired) {
    destroy_native_group_storage_00b8f680(owner, frame, context, acquired);
    if ((*reinterpret_cast<const volatile unsigned char*>(&flags) & 1u) != 0) {
        acquired.active_call_site = 0x00b8f8d5;
        pool.return_raw_slot_00b8ed40(owner);
        acquired.pool_returned = true;
    }
    return owner;
}

void release_native_group_storage_00b8eec0(void* owner,
    NativeNodeTreeRetirementFrame& frame, NativeNodeTreeRetirementContext& context,
    NativeGroupStorageReleaseAcquired& acquired) {
    if (acquired.started || acquired.node.started)
        throw std::invalid_argument("group release requires fresh persistent diagnostics");
    acquired.started = true;
    if (read(owner, 0x17c) != 0) {
        do {
            const Word count = read(owner, 0x17c);
            const Word begin = read(owner, 0x178);
            void* const back = reinterpret_cast<void*>(read(
                reinterpret_cast<void*>(begin + count * 4u - 4u)));
            if (count != 0) write(owner, 0x17c, count - 1u);
            write(back, 0xa0, 0);
            ++acquired.cleared_entries;
        } while (read(owner, 0x17c) != 0);
    }
    acquired.active_call_site = 0x00b8ef00;
    release_native_node_tree_00b6f310(owner, frame, context, acquired.node);
    acquired.complete = true; // metadata only: native tail may have freed owner
}

void __fastcall invalidate_native_group_world_00b8e6b0(void* owner) noexcept {
    write(owner, 0x138, read(owner, 0x138) & 0xffffffcfu);
}

NativeGroupStorageReference::NativeGroupStorageReference(void* owner,
    NativeGroupStorageIdentityContext& context, NativeGroupStorageLifetimeFrame& frame,
    NativeGroupStorageLifetimeAcquired& acquired)
    : RenderCommandReference(bind_count(owner, context, acquired)), identity_(owner),
      context_(context), frame_(frame), acquired_(acquired) {
    // Insertion may allocate/throw. Do it BEFORE the noexcept, byte-preserving
    // descriptor lifetime transition, so failure leaves the old view live.
    context_.owners.bind(owner, *this);
    prepare_descriptor(owner);
    acquired_.descriptor_prepared = true;
}
NativeGroupStorageReference::~NativeGroupStorageReference() {
    if (phase_ != Phase::retired) std::terminate();
}
bool NativeGroupStorageReference::retired() const noexcept { return phase_ == Phase::retired; }
void NativeGroupStorageReference::retire() noexcept {
    phase_ = Phase::retired;
    context_.owners.unbind(identity_, *this);
}
void* NativeGroupStorageReference::delete_scalar_00b8f8c0(const volatile Word& flags) {
    if (phase_ != Phase::bound)
        throw std::logic_error("group scalar deletion requires its bound companion");
    phase_ = Phase::destroying;
    void* result;
    try { result = delete_native_group_storage_00b8f8c0(identity_, flags,
        context_.pool_010902f4, frame_, context_.lifetime, acquired_); }
    catch (...) { retire(); throw; }
    retire();
    return result;
}
void NativeGroupStorageReference::release_zero_references() noexcept {
    if (phase_ != Phase::bound || reference_count.load(std::memory_order_relaxed) != 0)
        std::terminate();
    try {
        require_slot(read(identity_), context_, 0, 0x00bd30e0);
        GroupDeleteCalls calls(identity_, *this, context_);
        invoke_native_ref_counted_delete_00bd30e0(identity_, calls);
    } catch (...) { std::terminate(); }
}
} // namespace bsp
