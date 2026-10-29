#include "bsp/native_model_storage_lifetime.hpp"
#include "bsp/native_ref_counted.hpp"
#include <atomic>
#include <cstddef>
#include <exception>
#include <new>
#include <stdexcept>
#include <type_traits>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Raw Model storage lifetime requires MSVC Win32.
#endif

namespace bsp {
namespace {
using Word = std::uint32_t;
static_assert(sizeof(void*) == 4);
static_assert(sizeof(NativeNodeStorage) == 0x174);
static_assert(offsetof(NativeNodeStorage, references_04) == 4);
struct ModelTailWords { Word retained_174, scalar_178, scalar_17c, geometry_180; };
static_assert(sizeof(ModelTailWords) == 0x10);
static_assert(offsetof(ModelTailWords, geometry_180) == 0x0c);
static_assert(std::is_trivially_destructible_v<ModelTailWords>);
void* at(void* p, Word offset) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<Word>(p) + offset);
}
Word read(const void* p, Word offset = 0) noexcept {
    return *reinterpret_cast<const volatile Word*>(reinterpret_cast<Word>(p) + offset);
}
void write(void* p, Word offset, Word value) noexcept {
    *static_cast<volatile Word*>(at(p, offset)) = value;
}
void* pointer(const void* p, Word offset) noexcept {
    return reinterpret_cast<void*>(read(p, offset));
}
void require_context(const NativeModelStorageContext& c) {
    if (&c.node.trees.owners != static_cast<NativeRenderActualOwners*>(&c.node.scenes.owners) ||
        &c.node.scenes.decrement_00ce2220 != &c.node.trees.decrement_00ce2220)
        throw std::invalid_argument("raw Model requires SAME actual owners and decrement cell");
}
void require_live_slot(void* owner, NativeModelPool& pool) {
    if (!owner || reinterpret_cast<Word>(owner) % 4u != 0)
        throw std::invalid_argument("raw Model requires actual aligned188h pool storage");
    const auto& storage = pool.storage();
    const Word slab_index = NativeModelPool::live_slab_index(owner);
    if (!storage.slabs_28 || slab_index >= storage.slab_count_2c)
        throw std::invalid_argument("raw Model requires SAME allocated pool slab");
    const auto* const slab = storage.slabs_28[slab_index];
    const Word offset = reinterpret_cast<Word>(owner) - reinterpret_cast<Word>(slab);
    if (!slab || offset >= NativeModelPool::slot_bytes * NativeModelPool::slots_per_slab ||
        offset % NativeModelPool::slot_bytes != 0)
        throw std::invalid_argument("raw Model requires exact SAME slab slot identity");
    const Word count = *reinterpret_cast<const volatile std::uint16_t*>(slab + 0x3140);
    if (count > NativeModelPool::slots_per_slab)
        throw std::invalid_argument("raw Model pool has invalid free-count");
    const auto* const free_slots = reinterpret_cast<const volatile std::uint16_t*>(slab + 0x3100);
    for (Word index = 0; index < count; ++index) {
        if (free_slots[index] >= NativeModelPool::slots_per_slab)
            throw std::invalid_argument("raw Model pool has invalid free-index");
        if (free_slots[index] == offset / NativeModelPool::slot_bytes)
            throw std::invalid_argument("raw Model cannot use a returned pool slot");
    }
}
void require_fresh(const NativeModelStorageLifetimeAcquired& a) {
    if (a.started || a.complete || a.node.started || a.geometry_unwind.started)
        throw std::invalid_argument("raw Model destruction requires fresh persistent diagnostics");
}
void require_target(NativeTextureSurfaceReferenceIncrement target) {
    if (!target) throw std::logic_error("raw Model requires the genuine reached decrement target");
}
void terminal(void* captured, NativeModelStorageContext& c) {
    // The existing owner contract supplies the genuine CURRENT0 family policy.
    // No model/node extent is assumed for an arbitrary retained owner.
    auto& reference = c.node.scenes.owners.resolve_actual(captured);
    if (static_cast<void*>(&reference.reference_count) != at(captured, 4) ||
        reference.reference_count.load(std::memory_order_relaxed) != 0)
        throw std::logic_error("raw Model member terminal requires SAME observed-zero actual count");
    reference.release_zero_references();
}
void require_slot(Word profile, NativeModelStorageIdentityContext& c,
    unsigned slot, Word target) {
    if (profile != 0x00d62de8 || !c.actual_profile_00d62de8 ||
        c.actual_profile_00d62de8[slot] != target)
        throw std::logic_error("raw Model lacks its genuine current native slot");
}
std::atomic<std::int32_t>& bind_count(void* owner, NativeModelStorageIdentityContext& c,
    NativeModelStorageConstructAcquired& construction,
    const NativeModelStorageLifetimeAcquired& lifetime) {
    require_context(c.storage);
    require_live_slot(owner, c.storage.pool_01090054);
    require_fresh(lifetime);
    if (lifetime.scalar_started || !construction.complete || !construction.node_constructor_returned ||
        !construction.tail_constructed || construction.metadata_bound ||
        construction.actual_model != owner || construction.context != &c.storage ||
        c.storage.node.scenes.owners.find(owner))
        throw std::invalid_argument("raw Model admission requires its same completed unregistered constructor");
    require_slot(read(owner), c, 0, 0x00bd30e0);
    require_slot(read(owner), c, 1, 0x00b75290);
    auto& count = std::launder(static_cast<NativeNodeStorage*>(owner))->references_04;
    if (count.load(std::memory_order_relaxed) <= 0)
        throw std::invalid_argument("raw Model admission requires live actual+4");
    return count;
}
class ModelDeleteCalls final : public NativeRefCountedDeleteCalls {
public:
    ModelDeleteCalls(void* owner, NativeModelStorageReference& reference,
        NativeModelStorageIdentityContext& c, NativeModelStorageLifetimeFrame& frame)
        : owner_(owner), reference_(reference), context_(c), frame_(frame) {}
    void delete_vslot04(void* owner, Word profile, Word flags) override {
        if (owner != owner_ || flags != 1)
            throw std::logic_error("raw Model terminal requires SAME owner and flags1");
        require_slot(profile, context_, 1, 0x00b75290);
        frame_.scalar_flags_argument = flags;
        reference_.delete_scalar_00b75290(frame_.scalar_flags_argument);
    }
private:
    void* const owner_;
    NativeModelStorageReference& reference_;
    NativeModelStorageIdentityContext& context_;
    NativeModelStorageLifetimeFrame& frame_;
};
void destroy_model(void* owner, NativeModelStorageLifetimeFrame& frame,
    NativeModelStorageContext& c, NativeModelStorageLifetimeAcquired& a) {
    require_context(c);
    require_live_slot(owner, c.pool_01090054);
    require_fresh(a);
    a.started = true;
    int state = -1;
    try {
        write(owner, 0, 0x00d62de8);
        void* const retained = pointer(owner, 0x174); // B750E5 BEFORE import epoch.
        a.captured_retained = retained;
        const auto decrement = c.node.scenes.decrement_00ce2220; // B750ED, ONCE.
        a.captured_decrement = decrement;
        state = a.native_eh_state = 1;
        if (retained) {
            require_target(decrement);
            a.active_call_site = 0x00b75101;
            const long count = decrement(static_cast<volatile long*>(at(retained, 4)));
            a.retained_decrement_completed = true;
            if (count == 0) {
                a.active_call_site = 0x00b7510d;
                terminal(retained, c);
                a.retained_terminal_completed = true;
            }
            write(owner, 0x174, 0); // AFTER callback, even if it replaced174.
            a.retained_cleared = true;
        }
        void* const geometry = pointer(owner, 0x180); // CURRENT after174 callback.
        a.captured_geometry = geometry;
        state = a.native_eh_state = 0; // consume member cleanup BEFORE call.
        if (geometry) {
            require_target(decrement);
            a.active_call_site = 0x00b7512c;
            const long count = decrement(static_cast<volatile long*>(at(geometry, 4)));
            a.geometry_decrement_completed = true;
            if (count == 0) {
                a.active_call_site = 0x00b75138;
                terminal(geometry, c);
                a.geometry_terminal_completed = true;
            }
            write(owner, 0x180, 0);
            a.geometry_cleared = true;
        }
        state = a.native_eh_state = -1;
        a.active_call_site = 0x00b7514e;
        destroy_native_raw_node_base_00b6f440(owner, frame.node, c.node, a.node);
        a.complete = true;
    } catch (...) {
        a.exception_cleanup_started = true;
        try {
            while (state >= 0) {
                const int action = state--;
                a.native_eh_state = state; // consume before the cleanup call.
                if (action == 1) {
                    a.active_call_site = 0x00cc1c61;
                    destroy_native_model_geometry_storage_00b74f20(at(owner, 0x178), c,
                        a.geometry_unwind); // FRESH current CE2220, not saved epoch.
                } else {
                    a.active_call_site = 0x00cc1c53;
                    destroy_native_raw_node_base_00b6f440(owner, frame.node, c.node, a.node);
                }
            }
        } catch (...) { std::terminate(); }
        throw;
    }
}
} // namespace

void* construct_native_model_storage_00b75030(void* owner, const void* name,
    NativeModelStorageContext& c, NativeModelStorageConstructAcquired& a) {
    if (a.started || a.complete || a.metadata_bound || a.actual_model || a.context)
        throw std::invalid_argument("raw Model construction requires fresh persistent diagnostics");
    require_context(c);
    require_live_slot(owner, c.pool_01090054);
    a.started = true;
    a.actual_model = owner;
    a.context = &c;
    a.active_call_site = 0x00b75038;
    construct_native_node_00b6f5a0(owner, NativeModelPool::slot_bytes, name,
        c.node.strings, c.constants);
    a.node_constructor_returned = true;
    // Trivial Source tail lifetime only; default initialization writes no bytes.
    (void)::new (at(owner, 0x174)) ModelTailWords;
    a.tail_constructed = true;
    const Word minimum = c.constants.negative_bound_00ce4adc; // B7503D.
    write(owner, 0, 0x00d62de8);
    write(owner, 0x174, 0);
    write(owner, 0x178, minimum);
    write(owner, 0x17c, c.constants.positive_bound_00ce4970);
    const Word negative_zero = c.negative_zero_00d7a208;
    write(owner, 0x180, 0);
    write(owner, 0x18, negative_zero);
    write(owner, 0x1c, negative_zero);
    write(owner, 0x20, negative_zero);
    write(owner, 0x24, 0); write(owner, 0x28, 0); write(owner, 0x2c, 0);
    write(owner, 8, 0); write(owner, 0x0c, 0); write(owner, 0x10, 0); write(owner, 0x14, 0);
    a.complete = true;
    return owner;
}

void destroy_native_model_geometry_storage_00b74f20(void* member,
    NativeModelStorageContext& c, NativeModelStorageMemberAcquired& a) {
    if (a.started || a.complete)
        throw std::invalid_argument("raw Model member cleanup requires fresh persistent diagnostics");
    require_context(c);
    if (!member || reinterpret_cast<Word>(member) < 0x178u)
        throw std::invalid_argument("raw Model member requires actual model+178");
    require_live_slot(reinterpret_cast<void*>(reinterpret_cast<Word>(member) - 0x178u), c.pool_01090054);
    a.started = true;
    void* const captured = pointer(member, 8);
    a.captured_owner = captured;
    if (captured) {
        const auto decrement = c.node.scenes.decrement_00ce2220; // FRESH B74F2F.
        a.captured_decrement = decrement;
        require_target(decrement);
        a.active_call_site = 0x00b74f2f;
        const long count = decrement(static_cast<volatile long*>(at(captured, 4)));
        a.decrement_completed = true;
        if (count == 0) {
            a.active_call_site = 0x00b74f3f;
            terminal(captured, c);
            a.terminal_completed = true;
        }
        write(member, 8, 0);
        a.field_cleared = true;
    }
    a.complete = true;
}

void destroy_native_model_storage_00b750c0(void* owner,
    NativeModelStorageLifetimeFrame& frame, NativeModelStorageContext& c,
    NativeModelStorageLifetimeAcquired& a) {
    if (a.scalar_started)
        throw std::invalid_argument("raw Model direct destruction cannot resume scalar deletion");
    destroy_model(owner, frame, c, a);
}

void* delete_native_model_storage_00b75290(void* owner, const volatile Word& flags,
    NativeModelStorageLifetimeFrame& frame, NativeModelStorageContext& c,
    NativeModelStorageLifetimeAcquired& a) {
    require_fresh(a);
    if (a.scalar_started || a.scalar_complete)
        throw std::invalid_argument("raw Model scalar deletion requires fresh persistent diagnostics");
    a.scalar_started = true;
    destroy_model(owner, frame, c, a);
    a.active_call_site = 0x00b75298;
    const auto low_flags = *reinterpret_cast<const volatile unsigned char*>(&flags);
    a.flags_low_byte = low_flags;
    a.flags_read = true;
    if ((low_flags & 1u) != 0) {
        a.active_call_site = 0x00b752a5;
        c.pool_01090054.return_raw_slot_00b74750(owner);
        a.pool_returned = true;
    }
    a.scalar_complete = true;
    return owner; // numeric identity only after possible pool return.
}

NativeModelStorageReference::NativeModelStorageReference(void* owner,
    NativeModelStorageIdentityContext& c, NativeModelStorageConstructAcquired& construction,
    NativeModelStorageLifetimeFrame& frame, NativeModelStorageLifetimeAcquired& acquired)
    : RenderCommandReference(bind_count(owner, c, construction, acquired)), identity_(owner),
      context_(c), frame_(frame), acquired_(acquired) {
    c.storage.node.scenes.owners.bind(owner, *this);
    construction.metadata_bound = true;
}
NativeModelStorageReference::~NativeModelStorageReference() {
    if (phase_ != Phase::retired) std::terminate();
}
bool NativeModelStorageReference::retired() const noexcept { return phase_ == Phase::retired; }
void NativeModelStorageReference::retire() noexcept {
    phase_ = Phase::retired;
    context_.storage.node.scenes.owners.unbind(identity_, *this);
}
void* NativeModelStorageReference::delete_scalar_00b75290(const volatile Word& flags) {
    if (phase_ != Phase::bound)
        throw std::logic_error("raw Model scalar deletion requires its bound canonical companion");
    phase_ = Phase::destroying;
    void* result;
    try { result = delete_native_model_storage_00b75290(identity_, flags, frame_, context_.storage, acquired_); }
    catch (...) { retire(); throw; }
    retire();
    return result;
}
void NativeModelStorageReference::release_zero_references() noexcept {
    if (phase_ != Phase::bound || reference_count.load(std::memory_order_relaxed) != 0)
        std::terminate();
    try {
        require_slot(read(identity_), context_, 0, 0x00bd30e0);
        ModelDeleteCalls calls(identity_, *this, context_, frame_);
        invoke_native_ref_counted_delete_00bd30e0(identity_, calls);
    } catch (...) { std::terminate(); }
}
} // namespace bsp
