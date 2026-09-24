#include "bsp/native_camera_storage_lifetime.hpp"
#include "bsp/native_node_construction.hpp"
#include "bsp/native_ref_counted.hpp"
#include "bsp/native_viewport_owner.hpp"
#include "bsp/system_fog_owner.hpp"
#include <exception>
#include <stdexcept>

namespace bsp {
namespace {
using Word = std::uint32_t;
Word bits(const void* p) noexcept { return reinterpret_cast<Word>(p); }
void* ptr(Word p) noexcept { return reinterpret_cast<void*>(p); }
Word read(const void* p, Word n = 0) noexcept {
    return *reinterpret_cast<const volatile Word*>(bits(p) + n);
}
void write(void* p, Word n, Word value) noexcept {
    *reinterpret_cast<volatile Word*>(bits(p) + n) = value;
}
void require_same_domain(const NativeCameraStorageLifetimeContext& c) {
    if (&c.increment_00ce221c != &c.node.scenes.increment_00ce221c ||
        &c.decrement_00ce2220 != &c.node.scenes.decrement_00ce2220 ||
        &c.decrement_00ce2220 != &c.node.trees.decrement_00ce2220 ||
        static_cast<NativeRenderActualOwners*>(&c.node.scenes.owners) != &c.node.trees.owners)
        throw std::invalid_argument("camera lifetime requires SAME registry and current import cells");
}
enum class Family { viewport, fog };
const volatile Word* family_table(Word profile, Family family,
    NativeCameraStorageLifetimeContext& c) {
    const volatile Word* table = nullptr;
    if (family == Family::viewport && profile == 0x00d5e5f8) table = c.viewport_profile_00d5e5f8;
    if (family == Family::fog && profile == 0x00d63180) table = c.fog_profile_00d63180;
    if (!table) throw std::invalid_argument("unadmitted current viewport/fog family table");
    return table;
}
class FamilyDelete final : public NativeRefCountedDeleteCalls {
public:
    FamilyDelete(Family f, NativeCameraStorageLifetimeContext& c) : family_(f), context_(c) {}
    void delete_vslot04(void* actual, Word current_profile, Word flags) override {
        const Word target = family_table(current_profile, family_, context_)[1];
        if (family_ == Family::viewport && target == 0x00b1f8f0) {
            (void)delete_native_viewport_owner_00b1f8f0(static_cast<NativeViewportOwner*>(actual), flags);
        } else if (family_ == Family::fog && target == 0x00b84f70) {
            (void)delete_system_fog_owner_00b84f70(static_cast<SystemFogOwner*>(actual), flags);
        } else throw std::invalid_argument("unadmitted fresh viewport/fog current4");
    }
private:
    Family family_;
    NativeCameraStorageLifetimeContext& context_;
};
void family_zero(void* captured, Family family, NativeCameraStorageLifetimeContext& c) {
    // Concrete actual34h/94h lifetime and allocation provenance are preconditions;
    // both owners have a real volatile long at+4, no atomic/registry projection.
    if (*reinterpret_cast<const volatile long*>(bits(captured) + 4u) != 0)
        throw std::logic_error("viewport/fog terminal requires observed actual+4 zero");
    const Word profile = read(captured);
    if (family_table(profile, family, c)[0] != 0x00bd30e0)
        throw std::invalid_argument("unadmitted current viewport/fog zero target");
    FamilyDelete calls(family, c);
    invoke_native_ref_counted_delete_00bd30e0(captured, calls); // fresh profile/4
}
void retained_zero(void* captured, NativeCameraStorageLifetimeContext& c) {
    const Word profile = read(captured);
    const auto* table = c.retained.resolve_profile(profile);
    if (!table) throw std::invalid_argument("retained438 requires its actual current profile table");
    const Word target = table[0];
    c.retained.invoke_virtual0(target, captured); // family/extent/zero admitted by provider
}
void release_family(void* captured, NativeTextureSurfaceReferenceIncrement decrement,
    Family family, NativeCameraStorageLifetimeContext& c) {
    if (decrement(reinterpret_cast<volatile long*>(bits(captured) + 4u)) == 0)
        family_zero(captured, family, c);
}
void release_retained(void* captured, NativeTextureSurfaceReferenceIncrement decrement,
    NativeCameraStorageLifetimeContext& c) {
    if (decrement(reinterpret_cast<volatile long*>(bits(captured) + 4u)) == 0)
        retained_zero(captured, c);
}
void require_camera_slot(Word current_profile, Word offset, Word expected,
    NativeCameraStorageLifetimeContext& c) {
    if (current_profile != 0x00d62cf0 || !c.camera_profile_00d62cf0 ||
        c.camera_profile_00d62cf0[offset / 4] != expected)
        throw std::invalid_argument("unadmitted current camera lifetime profile/target");
}
void require_live_pool_slot(void* actual, std::size_t extent, NativeCameraPool& pool) {
    if (extent < NativeCameraPool::slot_bytes || (bits(actual) & 3u) != 0)
        throw std::invalid_argument("camera companion requires actual aligned45Ch pool slot");
    const auto& storage = pool.storage();
    const Word index = NativeCameraPool::live_slab_index(actual);
    if (!storage.slabs_28 || index >= storage.slab_count_2c)
        throw std::invalid_argument("camera companion requires SAME allocated pool slab");
    const auto* slab = storage.slabs_28[index];
    const Word offset = bits(actual) - bits(slab);
    if (!slab || offset >= NativeCameraPool::slot_bytes * NativeCameraPool::slots_per_slab ||
        offset % NativeCameraPool::slot_bytes != 0)
        throw std::invalid_argument("camera companion requires actual slab slot identity");
    const Word count = *reinterpret_cast<const volatile std::uint16_t*>(slab + 0x8bc0);
    if (count > NativeCameraPool::slots_per_slab)
        throw std::invalid_argument("invalid actual camera slab free-count");
    const auto* free_slots = reinterpret_cast<const volatile std::uint16_t*>(slab + 0x8b80);
    for (Word i = 0; i < count; ++i)
        if (free_slots[i] == offset / NativeCameraPool::slot_bytes)
            throw std::invalid_argument("camera companion cannot admit returned slot");
}
class CameraDelete final : public NativeRefCountedDeleteCalls {
public:
    CameraDelete(NativeCameraStorageReference& reference, NativeCameraStorageLifetimeContext& c)
        : reference_(reference), context_(c) {}
    void delete_vslot04(void*, Word current_profile, Word flags) override {
        require_camera_slot(current_profile, 4, 0x00b71fe0, context_);
        (void)reference_.delete_scalar_00b71fe0(flags);
    }
private:
    NativeCameraStorageReference& reference_;
    NativeCameraStorageLifetimeContext& context_;
};
}

void set_native_camera_viewport_storage_00b71990(void* actual,
    const volatile Word& incoming_argument, NativeCameraStorageLifetimeContext& c) {
    require_same_domain(c);
    const Word incoming = incoming_argument; // B71990 BEFORE old180
    const Word old = read(actual, 0x180);
    if (old == incoming) return;
    write(actual, 0x180, incoming);
    if (incoming) c.increment_00ce221c(reinterpret_cast<volatile long*>(incoming + 4u));
    if (old) release_family(ptr(old), c.decrement_00ce2220, Family::viewport, c);
}
void clear_native_camera_retained_storage_00605fd0(volatile Word& actual_cell,
    NativeCameraStorageLifetimeContext& c) {
    require_same_domain(c);
    const Word captured = actual_cell;
    if (!captured) return;
    release_retained(ptr(captured), c.decrement_00ce2220, c); // FRESH current epoch
    actual_cell = 0; // after terminal, even when it replaced this cell
}
void destroy_native_camera_storage_00b71f10(void* actual,
    NativeCameraStorageDestroyFrame& frame, NativeCameraStorageLifetimeContext& c,
    NativeCameraStorageAcquired& acquired) {
    require_same_domain(c);
    if (acquired.started) throw std::invalid_argument("camera destruction requires fresh diagnostics");
    acquired.started = true;
    write(actual, 0, 0x00d62cf0);
    void* captured = ptr(read(actual, 0x180)); // BEFORE capturing epoch
    const auto decrement = c.decrement_00ce2220;
    acquired.captured_decrement = decrement;
    int state = 1;
    acquired.native_eh_state = state;
    try {
        if (captured) {
            acquired.active_call_site = 0x00b71f54;
            release_family(captured, decrement, Family::viewport, c);
            write(actual, 0x180, 0);
            acquired.viewport_cleared = true;
        }
        captured = ptr(read(actual, 0x184));
        if (captured) {
            acquired.active_call_site = 0x00b71f76;
            release_family(captured, decrement, Family::fog, c);
            write(actual, 0x184, 0);
            acquired.fog_cleared = true;
        }
        captured = ptr(read(actual, 0x438)); // BEFORE consuming retained cleanup
        state = acquired.native_eh_state = 0;
        if (captured) {
            acquired.active_call_site = 0x00b71f9c;
            release_retained(captured, decrement, c);
            write(actual, 0x438, 0);
            acquired.retained_cleared = true;
        }
        state = acquired.native_eh_state = -1;
        acquired.active_call_site = 0x00b71fba;
        destroy_native_raw_node_base_00b6f440(actual, frame.node, c.node, acquired.node);
        acquired.complete = true;
    } catch (...) {
        acquired.exception_cleanup_started = true;
        try {
            while (state >= 0) {
                const int action = state--;
                acquired.native_eh_state = state; // consume BEFORE call
                if (action == 1) {
                    acquired.active_call_site = 0x00cc1b31;
                    clear_native_camera_retained_storage_00605fd0(
                        *reinterpret_cast<volatile Word*>(bits(actual) + 0x438u), c);
                    acquired.retained_cleared = true;
                } else {
                    acquired.active_call_site = 0x00cc1b23;
                    destroy_native_raw_node_base_00b6f440(actual, frame.node, c.node, acquired.node);
                }
            }
        } catch (...) { std::terminate(); }
        throw;
    }
}
void* delete_native_camera_storage_00b71fe0(void* actual,
    const volatile Word& flags, NativeCameraStorageDestroyFrame& frame,
    NativeCameraStorageLifetimeContext& c, NativeCameraStorageAcquired& acquired) {
    destroy_native_camera_storage_00b71f10(actual, frame, c, acquired);
    if ((*reinterpret_cast<const volatile unsigned char*>(&flags) & 1u) != 0) {
        acquired.active_call_site = 0x00b71ff5;
        c.camera_pool_0108ffb0.return_raw_slot_00b711e0(actual);
        acquired.pool_returned = true; // metadata only after actual slot return
    }
    return actual;
}
NativeCameraStorageReference::NativeCameraStorageReference(NativeNodeStorage& prefix,
    std::size_t extent, NativeCameraStorageLifetimeContext& c,
    NativeCameraStorageDestroyFrame& frame, NativeCameraStorageAcquired& acquired)
    : RenderCommandReference(prefix.references_04), actual_(&prefix), context_(c),
      frame_(frame), acquired_(acquired) {
    require_same_domain(c);
    require_live_pool_slot(actual_, extent, c.camera_pool_0108ffb0);
    if (reference_count.load(std::memory_order_relaxed) <= 0 || acquired.started)
        throw std::invalid_argument("camera companion requires live count and fresh diagnostics");
    require_camera_slot(read(actual_), 0, 0x00bd30e0, c);
    require_camera_slot(read(actual_), 4, 0x00b71fe0, c);
    c.node.scenes.owners.bind(actual_, *this); // metadata only, no credit/native store
}
NativeCameraStorageReference::~NativeCameraStorageReference() {
    if (phase_ != Phase::retired) std::terminate();
}
bool NativeCameraStorageReference::retired() const noexcept { return phase_ == Phase::retired; }
void NativeCameraStorageReference::retire() noexcept {
    context_.node.scenes.owners.unbind(actual_, *this); // identity only, not payload
    phase_ = Phase::retired;
}
void* NativeCameraStorageReference::delete_scalar_00b71fe0(const volatile Word& flags) {
    if (phase_ != Phase::bound) throw std::logic_error("camera companion already retiring");
    phase_ = Phase::destroying;
    try {
        void* const result = delete_native_camera_storage_00b71fe0(actual_, flags, frame_, context_, acquired_);
        retire();
        return result;
    } catch (...) { retire(); throw; } // diagnostics remain; not success/rollback
}
void NativeCameraStorageReference::release_zero_references() noexcept {
    if (phase_ != Phase::bound || reference_count.load(std::memory_order_relaxed) != 0)
        std::terminate();
    try {
        require_camera_slot(read(actual_), 0, 0x00bd30e0, context_);
        CameraDelete calls(*this, context_);
        invoke_native_ref_counted_delete_00bd30e0(actual_, calls);
    } catch (...) { std::terminate(); }
}
} // namespace bsp
