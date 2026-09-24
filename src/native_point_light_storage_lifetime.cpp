#include "bsp/native_point_light_storage_lifetime.hpp"
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
void require_array(NativePointLightBacklinkArray& array, NativePointLightLinksRuntime& runtime) {
    if (array.count < 0 || array.capacity < array.count ||
        static_cast<Word>(array.capacity) > 0x3fffffffu ||
        (array.capacity != 0 && array.begin == nullptr))
        throw std::invalid_argument("point array requires valid actual descriptor extent");
    runtime.require_owned_backing(array.begin, array.capacity);
}
void require_context(void* owner, NativePointLightStorageContext& context) {
    auto& node = context.light.node;
    if (!owner || reinterpret_cast<Word>(owner) % 4 ||
        &node.scenes.owners != &node.trees.owners ||
        &node.scenes.decrement_00ce2220 != &node.trees.decrement_00ce2220 ||
        context.backlinks.identity != owner ||
        static_cast<void*>(&context.backlinks.backlinks) != at(owner, 0x1e0) ||
        &node.trees.point_lights.light(owner) != &context.backlinks)
        throw std::invalid_argument("point lifetime requires SAME actual backlink and owner domains");
    require_array(context.backlinks.backlinks, node.trees.point_lights);
}
void retire_backlink(NativePointLightStorageContext& context,
    NativePointLightStorageAcquired& acquired) {
    // Explicit host metadata boundary, while payload/count remain live. The
    // existing unbind reads count, so NEVER defer it until after pool return.
    context.light.node.trees.point_lights.unbind_light(context.backlinks);
    acquired.backlink_binding_retired = true;
}
void require_slot(Word profile, NativePointLightStorageIdentityContext& c,
    unsigned slot, Word target) {
    if (profile != 0x00d63008 || !c.actual_profile_00d63008 ||
        c.actual_profile_00d63008[slot] != target)
        throw std::logic_error("point light lacks its genuine current native slot");
}
void require_live_pool_slot(void* owner, NativePointLightPool& pool) {
    if (!owner || reinterpret_cast<Word>(owner) % 4)
        throw std::invalid_argument("point companion requires actual aligned200h pool slot");
    const auto& storage = pool.storage();
    const Word index = NativePointLightPool::live_slab_index(owner);
    if (!storage.slabs_28 || index >= storage.slab_count_2c)
        throw std::invalid_argument("point companion requires SAME allocated pool slab");
    const auto* slab = storage.slabs_28[index];
    const Word offset = reinterpret_cast<Word>(owner) - reinterpret_cast<Word>(slab);
    if (!slab || offset >= NativePointLightPool::slot_bytes * NativePointLightPool::slots_per_slab ||
        offset % NativePointLightPool::slot_bytes != 0)
        throw std::invalid_argument("point companion requires actual slab slot identity");
    const Word count = *reinterpret_cast<const volatile std::uint16_t*>(slab + 0x4040);
    if (count > NativePointLightPool::slots_per_slab)
        throw std::invalid_argument("invalid actual point slab free-count");
    const auto* free_slots = reinterpret_cast<const volatile std::uint16_t*>(slab + 0x4000);
    for (Word i = 0; i < count; ++i)
        if (free_slots[i] == offset / NativePointLightPool::slot_bytes)
            throw std::invalid_argument("point companion cannot admit returned slot");
}
std::atomic<std::int32_t>& bind_count(void* owner, NativePointLightStorageIdentityContext& c,
    NativePointLightStorageAcquired& acquired) {
    require_live_pool_slot(owner, c.pool_0109011c); // pure, BEFORE atomic/registry admission
    require_context(owner, c.lifetime);
    if (acquired.started || acquired.light.started ||
        &c.owners != &c.lifetime.light.node.scenes.owners || c.owners.find(owner))
        throw std::invalid_argument("point binding requires unregistered completed actual storage");
    require_slot(read(owner), c, 0, 0x00bd30e0);
    require_slot(read(owner), c, 1, 0x00b7c850);
    auto& count = *std::launder(reinterpret_cast<std::atomic<std::int32_t>*>(at(owner, 4)));
    if (count.load(std::memory_order_relaxed) <= 0)
        throw std::invalid_argument("point binding requires live actual+4");
    return count;
}
class PointDeleteCalls final : public NativeRefCountedDeleteCalls {
public:
    PointDeleteCalls(void* owner, NativePointLightStorageReference& reference,
        NativePointLightStorageIdentityContext& context)
        : owner_(owner), reference_(reference), context_(context) {}
    void delete_vslot04(void* owner, Word profile, Word flags) override {
        if (owner != owner_ || flags != 1)
            throw std::logic_error("point terminal requires same owner and flags1");
        require_slot(profile, context_, 1, 0x00b7c850);
        const volatile Word actual_flags = flags;
        reference_.delete_scalar_00b7c850(actual_flags);
    }
private:
    void* const owner_;
    NativePointLightStorageReference& reference_;
    NativePointLightStorageIdentityContext& context_;
};
}

void* construct_native_point_light_raw_00b7c710(void* owner, std::size_t bytes,
    const void* name, NativeStringRawPoolContext& strings,
    const NativeNodeRawConstants& constants, const volatile Word& sixty_four) {
    if (!owner || reinterpret_cast<Word>(owner) % 4 || bytes < NativePointLightPool::slot_bytes)
        throw std::invalid_argument("raw point construction requires its actual aligned200h slot");
    construct_native_light_raw_00b7c4c0(owner, bytes, name, strings, constants, sixty_four);
    // Assembly-only Light tail stores precede these source descriptor
    // lifetimes. Materialize only12B scene metadata with all bytes preserved.
    static_assert(sizeof(SystemAmbientBacklinks) == 12);
    static_assert(std::is_trivially_copyable_v<SystemAmbientBacklinks>);
    std::byte preimage[12];
    std::memcpy(preimage, at(owner, 0x178), sizeof preimage);
    auto* scenes = ::new (at(owner, 0x178)) SystemAmbientBacklinks;
    std::memcpy(scenes, preimage, sizeof preimage);
    write(owner, 0, 0x00d63008);
    initialize_native_point_light_backlinks_00b7c710_fragment(owner, bytes);
    return owner;
}

void destroy_native_point_light_array_storage_005a1610(
    NativePointLightBacklinkArray& array, NativePointLightLinksRuntime& runtime) {
    require_array(array, runtime); // pure valid-domain admission before stores
    // Exact reached59FCE0(0) path: valid nonnegative capacity never reserves.
    while (read(&array, 4) != 0) write(&array, 4, read(&array, 4) - 1u);
    write(&array, 4, 0);
    void* const current_begin = reinterpret_cast<void*>(read(&array));
    runtime.free_backing(current_begin);
}

void destroy_native_point_light_storage_00b7c770(void* owner,
    NativePointLightStorageFrame& frame, NativePointLightStorageContext& context,
    NativePointLightStorageAcquired& acquired) {
    if (acquired.started || acquired.light.started)
        throw std::invalid_argument("point destruction requires fresh persistent diagnostics");
    require_context(owner, context);
    acquired.started = true;
    int state = -1;
    try {
        write(owner, 0, 0x00d63008);
        state = acquired.native_eh_state = 1;
        acquired.active_call_site = 0x00b7c79c;
        unlink_native_point_light_nodes_00b7c160(context.backlinks);
        acquired.backlinks_unlinked = true;
        state = acquired.native_eh_state = 0; // B7C7AB, BEFORE resize/free
        acquired.active_call_site = 0x00b7c7b0;
        destroy_native_point_light_array_storage_005a1610(
            context.backlinks.backlinks, context.light.node.trees.point_lights);
        acquired.array_cleanup_completed = true;
        retire_backlink(context, acquired); // metadata only, not native CALL
        state = acquired.native_eh_state = -1; // B7C7C2, BEFORE raw Light
        acquired.active_call_site = 0x00b7c7ca;
        destroy_native_light_storage_00b7c5b0(owner, frame.light, context.light, acquired.light);
        acquired.complete = true;
    } catch (...) {
        acquired.exception_cleanup_started = true;
        try {
            while (state >= 0) {
                const int current = state--;
                acquired.native_eh_state = state;
                if (current == 1) {
                    destroy_native_point_light_array_storage_005a1610(
                        context.backlinks.backlinks, context.light.node.trees.point_lights);
                    acquired.array_cleanup_completed = true;
                    retire_backlink(context, acquired);
                } else {
                    destroy_native_light_storage_00b7c5b0(owner, frame.light,
                        context.light, acquired.light);
                }
            }
        } catch (...) { std::terminate(); }
        throw;
    }
}

void* delete_native_point_light_storage_00b7c850(void* owner,
    const volatile Word& flags, NativePointLightPool& pool,
    NativePointLightStorageFrame& frame, NativePointLightStorageContext& context,
    NativePointLightStorageAcquired& acquired) {
    destroy_native_point_light_storage_00b7c770(owner, frame, context, acquired);
    if ((*reinterpret_cast<const volatile unsigned char*>(&flags) & 1u) != 0) {
        acquired.active_call_site = 0x00b7c865;
        pool.return_raw_slot_00b7b1d0(owner);
        acquired.pool_returned = true;
    }
    return owner;
}

NativePointLightStorageReference::NativePointLightStorageReference(void* owner,
    NativePointLightStorageIdentityContext& context, NativePointLightStorageFrame& frame,
    NativePointLightStorageAcquired& acquired)
    : RenderCommandReference(bind_count(owner, context, acquired)), identity_(owner),
      context_(context), frame_(frame), acquired_(acquired) {
    context_.owners.bind(owner, *this);
}
NativePointLightStorageReference::~NativePointLightStorageReference() {
    if (phase_ != Phase::retired) std::terminate();
}
bool NativePointLightStorageReference::retired() const noexcept { return phase_ == Phase::retired; }
void NativePointLightStorageReference::retire() noexcept {
    phase_ = Phase::retired;
    context_.owners.unbind(identity_, *this); // map identity only, no raw reads
}
void* NativePointLightStorageReference::delete_scalar_00b7c850(const volatile Word& flags) {
    if (phase_ != Phase::bound)
        throw std::logic_error("point scalar deletion requires its bound companion");
    phase_ = Phase::destroying;
    void* result;
    try { result = delete_native_point_light_storage_00b7c850(identity_, flags,
        context_.pool_0109011c, frame_, context_.lifetime, acquired_); }
    catch (...) { retire(); throw; }
    retire();
    return result;
}
void NativePointLightStorageReference::release_zero_references() noexcept {
    if (phase_ != Phase::bound || reference_count.load(std::memory_order_relaxed) != 0)
        std::terminate();
    try {
        require_slot(read(identity_), context_, 0, 0x00bd30e0);
        PointDeleteCalls calls(identity_, *this, context_);
        invoke_native_ref_counted_delete_00bd30e0(identity_, calls);
    } catch (...) { std::terminate(); }
}
} // namespace bsp
