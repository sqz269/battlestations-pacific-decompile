#include "bsp/native_matrix_storage_lifetime.hpp"
#include "bsp/native_matrix_indexed_model_owner.hpp"
#include "bsp/native_resource_instance_bones.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <atomic>
#include <cstddef>
#include <cstring>
#include <intrin.h>
#include <new>
#include <stdexcept>
#include <type_traits>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Raw Matrix lifetime requires MSVC Win32.
#endif
namespace bsp {
namespace {
using Word = std::uint32_t;
static_assert(sizeof(void*) == 4);
static_assert(sizeof(NativeRenderPointerArrayStorage) == 0x0c);
static_assert(std::is_trivially_default_constructible_v<NativeRenderPointerArrayStorage>);
static_assert(std::is_trivially_destructible_v<NativeRenderPointerArrayStorage>);
void* at(void* base, Word offset) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<Word>(base) + offset);
}
Word read_word(const void* address) noexcept {
    std::atomic_signal_fence(std::memory_order_seq_cst);
    Word value;
    std::memcpy(&value, address, sizeof value);
    std::atomic_signal_fence(std::memory_order_seq_cst);
    return value;
}
void write_word(void* address, Word value) noexcept {
    std::atomic_signal_fence(std::memory_order_seq_cst);
    std::memcpy(address, &value, sizeof value);
    std::atomic_signal_fence(std::memory_order_seq_cst);
}
struct Region { const void* pointer; std::size_t bytes; };
bool disjoint(Region a, Region b) noexcept {
    const auto left = static_cast<std::uint64_t>(reinterpret_cast<Word>(a.pointer));
    const auto right = static_cast<std::uint64_t>(reinterpret_cast<Word>(b.pointer));
    return left + a.bytes <= right || right + b.bytes <= left;
}
}
struct NativeMatrixStorageAccess {
    using Frame = NativeMatrixStorageFrame;
    using Phase = Frame::Phase;
    static void reject(Frame& f, const char* text) {
        f.poisoned_ = true;
        if (f.creator_.receiver) f.creator_.receiver->disposition_complete_ = false;
        throw std::logic_error(text);
    }
    static void require(const NativeMatrixStorageContext& c, Frame& f, Phase phase) {
        if (f.poisoned_ || f.phase_ != phase || (f.context_ && f.context_ != &c))
            reject(f, "Matrix operation/context cannot replay or reenter");
    }
    static void failed(Frame& f) noexcept {
        if (f.creator_.receiver) f.creator_.receiver->disposition_complete_ = false;
        if (!f.primary_) f.primary_ = std::current_exception();
        f.phase_ = Phase::failed;
    }
    static void healthy(Frame& f) {
        if (f.poisoned_) {
            f.phase_ = Phase::failed;
            throw std::logic_error("Matrix owning invocation was poisoned");
        }
    }
    static void backing(const NativeMatrixStorageContext& c, Frame& f) {
        const Region payload{f.actual_, NativeMatrixIndexedModelPool::slot_bytes};
        const Region records[] = {{&f, sizeof f}, {&c, sizeof c},
            {&c.prefix(), sizeof(c.prefix())},
            {&c.prefix().node(), sizeof(c.prefix().node())},
            {&c.prefix().node().scenes, sizeof(c.prefix().node().scenes)},
            {&c.prefix().node().trees, sizeof(c.prefix().node().trees)}};
        for (const auto record : records)
            if (!disjoint(payload, record))
                throw std::invalid_argument("Matrix full194 allocation overlaps retained metadata");
        for (std::size_t i = 1; i != sizeof(records) / sizeof(records[0]); ++i)
            if (!disjoint(records[0], records[i]))
                throw std::invalid_argument("Matrix frame overlaps retained context");
    }
    static void end_extents(Frame& f) {
        if (f.bones_live_) {
            f.bones_live_ = false; // consume before the trivial no-store end
            f.bones_->~NativeRenderPointerArrayStorage();
        }
        if (f.prefix_extent_.live())
            end_native_matrix_model_prefix_extent(f.prefix_extent_);
    }
    static void custody(const NativeMatrixStorageContext& c, Frame& f,
        NativeMatrixStorageOwner* receiver) {
        if (f.context_ != &c || (f.creator_.transferred ?
            !receiver || f.creator_.receiver != receiver || f.creator_.receiver_ended : receiver != nullptr))
            reject(f, "Matrix operation requires its exact untransferred/received Source custody");
    }
    static NativeNodeStorage& live_creator(const NativeMatrixStorageContext& c, Frame& f,
        NativeMatrixStorageOwner* receiver) {
        require(c, f, Phase::live);
        custody(c, f, receiver);
        const auto& credit = f.creator_;
        if (!credit.issued || !f.constructor_returned_ || !f.prefix_extent_.live() ||
            !f.bones_live_ || credit.release_entered || credit.decrement_completed ||
            credit.disposition_entered || f.destructor_entered_ || f.pool_return_entered_ ||
            credit.node != f.actual_ || !credit.count)
            reject(f, "Matrix creator borrow requires its private returned/live/undischarged facts");
        (void)admit_native_matrix_model_allocation(f.actual_, c.prefix());
        backing(c, f);
        // This is the ACTUAL Node174 born by the private prefix constructor,
        // never NativeModelOwner/NativeModelTailStorage over the private tail.
        auto& node = *credit.node;
        if (credit.count != &node.references_04 ||
            static_cast<void*>(credit.count) != at(f.actual_, 4) ||
            credit.count->load(std::memory_order_relaxed) <= 0 ||
            read_word(f.actual_) != 0x00d63590)
            reject(f, "Matrix creator borrow lost its SAME live count/allocation");
        return node;
    }
    static void transfer(const NativeMatrixStorageContext& c, Frame& f,
        NativeMatrixStorageOwner& receiver) {
        (void)live_creator(c, f, nullptr);
        if (f.geometry_depth_ || !disjoint({&receiver, sizeof receiver}, {&f, sizeof f}) ||
            !disjoint({&receiver, sizeof receiver}, {&c, sizeof c}) ||
            !disjoint({&receiver, sizeof receiver}, {f.actual_, NativeMatrixIndexedModelPool::slot_bytes}))
            reject(f, "Matrix creator move requires fresh disjoint stable receiver backing");
        f.creator_.receiver = &receiver;
        f.creator_.receiver_identity = &receiver; // opaque history survives legal C++ receiver end
        f.creator_.transferred = true; // Source custody only, NO Native retain/publication
    }
    static void* current_scene(const NativeMatrixStorageContext& c, Frame& f,
        NativeMatrixStorageOwner* receiver) {
        auto& node = live_creator(c, f, receiver);
        SceneResource* captured;
        _ReadWriteBarrier();
        std::memcpy(&captured, &node.scene_170, 4);
        _ReadWriteBarrier();
        return static_cast<void*>(captured); // opaque actual identity, not semantic permission
    }
    static void geometry(const NativeMatrixStorageContext& c, Frame& f,
        NativeMatrixStorageOwner* receiver, const NativeMatrixStorageGeometryContext& services,
        NativeModelStorageGeometryFrame& arguments, NativeModelStorageGeometryAcquired& acquired) {
        NativeModelGeometryKernelAccess::require_fresh(arguments, acquired);
        (void)live_creator(c, f, receiver);
        if (services.matrix_ != &c || !services.roles_ || f.geometry_depth_ == 0xffffffffu)
            reject(f, "Matrix geometry requires its genuine checked caller service binding");
        ++f.geometry_depth_;
        try {
            NativeModelGeometryKernelAccess::execute(f.actual_, arguments, c.prefix().node(),
                services.roles_->unchanged_00d7a260, acquired,
                receiver ? static_cast<const void*>(receiver) : static_cast<const void*>(&f),
                &c, &services, nullptr);
        } catch (...) {
            --f.geometry_depth_;
            f.poisoned_ = true;
            failed(f);
            throw;
        }
        --f.geometry_depth_;
        if (f.poisoned_) { // AFTER the whole Native body; retained metadata only
            try { NativeModelGeometryKernelAccess::reject_returned(arguments, acquired); }
            catch (...) { failed(f); throw; }
        }
    }
    static void release_creator(const NativeMatrixStorageContext& c, Frame& f,
        NativeMatrixStorageOwner* receiver, NativeNodeTreeRetirementFrame& arguments) {
        auto& node = live_creator(c, f, receiver);
        if (f.geometry_depth_ || !disjoint({&arguments, sizeof arguments}, {&f, sizeof f}) ||
            !disjoint({&arguments, sizeof arguments}, {f.actual_, NativeMatrixIndexedModelPool::slot_bytes}))
            reject(f, "Matrix creator release requires stable disjoint actual argument backing");
        f.creator_release_frame_ = &arguments; // no argument HOME read here
        f.creator_.release_entered = true;
        f.creator_.release_active = true;
        try {
            release_native_node_tree_00b6f310(&node, arguments, c.prefix().node().trees,
                f.creator_release_);
            // Actual CE2220 completion, not entry/progress/count or a caller-
            // supplied receipt. B6F310 sets it BEFORE count0 terminal dispatch.
            f.creator_.decrement_completed = f.creator_release_.decrement_completed;
            f.creator_.release_returned = true;
            f.creator_.release_active = false;
            healthy(f);
        } catch (...) {
            f.creator_.decrement_completed = f.creator_release_.decrement_completed;
            f.creator_.release_active = false;
            failed(f); // preserve observed decrement even if genuine count0 dispatch escaped
            throw;
        }
    }
    static void settled(NativeMatrixStorageOwner& owner, Frame& f) noexcept {
        if (f.phase_ == Phase::returned && f.pool_returned_ && !f.poisoned_ &&
            !f.creator_.release_active && !f.geometry_depth_)
            owner.disposition_complete_ = true;
    }
    static void end_receiver(NativeMatrixStorageOwner& owner, Frame& f) noexcept {
        // Frame/Context outlive this complete C++ destructor. No callback,
        // payload read, Native action, credit adjustment or cleanup occurs.
        if (!f.creator_.transferred || f.creator_.receiver != &owner || f.creator_.receiver_ended ||
            !owner.disposition_complete_ || f.phase_ != Phase::returned || !f.pool_returned_ ||
            f.poisoned_ || f.creator_.release_active || f.geometry_depth_)
            std::terminate();
        f.creator_.receiver = nullptr; // retire live authority BEFORE Owner C++ end completes
        f.creator_.receiver_ended = true; // transferred/history deliberately remain permanent
    }
    static void* construct(const void* name, const NativeMatrixStorageContext& c, Frame& f) {
        require(c, f, Phase::creating); // rejection outside factory cleanup ownership
        if (!f.allocation_returned_ || !f.actual_ || !f.factory_edge_ || f.constructor_entered_)
            reject(f, "Matrix constructor requires its genuine fresh allocation");
        f.constructor_entered_ = true;
        const auto allocation = admit_native_matrix_model_allocation(f.actual_, c.prefix());
        backing(c, f);
        construct_native_matrix_model_prefix_storage_00b75030(
            allocation, name, f.prefix_construct_, f.prefix_extent_);
        // This type was not born by a prepared NativeModelOwner. No initializer.
        f.bones_ = ::new (at(f.actual_, 0x184)) NativeRenderPointerArrayStorage;
        f.bones_live_ = true;
        write_word(f.actual_, 0x00d63590);
        write_word(at(f.actual_, 0x184), 0);
        write_word(at(f.actual_, 0x188), 0);
        write_word(at(f.actual_, 0x18c), 0);
        f.constructor_returned_ = true; // actual complete B91220 return facts
        f.creator_.node = std::launder(static_cast<NativeNodeStorage*>(f.actual_));
        f.creator_.count = &f.creator_.node->references_04;
        f.creator_.issued = true; // genuine count1 creator from this actual full constructor
        return f.actual_;
    }
    static void* create(const void* const volatile& home,
        const NativeMatrixStorageContext& c, Frame& f) {
        require(c, f, Phase::fresh); // no lower edge belongs to a rejected entry
        f.context_ = &c;
        f.phase_ = Phase::creating;
        f.allocation_entered_ = true;
        try {
            // B90C20 selects this SAME initialized01090390 owner; incoming190
            // allocation-size is not a storage-domain or copied extent proof.
            f.actual_ = c.prefix().pool_01090390().allocate_raw_slot_00b90ac0();
            f.allocation_returned_ = true;
            if (f.actual_) {
                f.factory_edge_ = true; // actual captured allocation state0
                const void* const name = home; // AFTER nonnull allocation only
                f.name_captured_ = true;
                construct_native_matrix_storage_00b91220(name, c, f);
                f.factory_edge_ = false; // actual ctor returned, BEFORE health fence
            }
        } catch (...) {
            f.primary_ = std::current_exception();
            if (f.factory_edge_) {
                f.factory_edge_ = false; // consume selected captured-slot action
                f.factory_return_entered_ = true;
                try {
                    // Genuine B90480 -> SAME pool B90150, no payload destructor.
                    c.prefix().pool_01090390().return_raw_slot_00b90150(f.actual_);
                    f.factory_return_returned_ = true;
                } catch (...) {
                    f.secondary_ = std::current_exception();
                    f.phase_ = Phase::failed;
                    std::terminate(); // qualified Source secondary projection only
                }
            }
            f.phase_ = Phase::failed;
            throw;
        }
        f.phase_ = f.actual_ ? Phase::live : Phase::null_result;
        healthy(f); // NULL still did not sample HOME; completed owner is not freed
        return f.actual_;
    }
    static void enter_destroy(const NativeMatrixStorageContext& c, Frame& f,
        NativeMatrixStorageOwner* receiver) {
        require(c, f, Phase::live);
        custody(c, f, receiver);
        if (f.geometry_depth_ || (f.creator_.release_active &&
            (!f.creator_release_.decrement_completed || f.creator_release_.active_call_site != 0x00b6f3ba)))
            reject(f, "Matrix destruction cannot invalidate an active live borrow/unfinished decrement");
        if (!f.constructor_returned_ || !f.prefix_extent_.live() || !f.bones_live_)
            reject(f, "Matrix destruction requires its actual completed constructor");
        f.phase_ = Phase::destroying;
        f.creator_.disposition_entered = true; // NOT a decrement receipt
        const auto allocation = admit_native_matrix_model_allocation(f.actual_, c.prefix());
        backing(c, f);
        static_cast<void>(allocation);
    }
    static void destroy_payload(const NativeMatrixStorageContext& c, Frame& f) {
        f.destructor_entered_ = true;
        f.base_edge_ = true; // B912A8, immediately before the member action
        f.resize_entered_ = true;
        try {
            resize_native_model_bone_nodes_00b90e30(f.actual_, 0);
            f.resize_returned_ = true;
            // CURRENT DATA after resize; free(NULL) is not suppressed.
            void* const current = reinterpret_cast<void*>(read_word(at(f.actual_, 0x184)));
            f.captured_bones_data_ = current;
            f.free_entered_ = true;
            singleton_lifetime_free(current);
            f.free_returned_ = true;
            f.base_edge_ = false; // B912C2 -1 BEFORE the normal base call
            f.normal_base_entered_ = true;
            destroy_native_matrix_model_prefix_storage_00b750c0(
                f.actual_, f.base_frame_, c.prefix(), f.base_);
            f.normal_base_returned_ = true;
        } catch (...) {
            f.primary_ = std::current_exception();
            if (f.base_edge_) {
                f.base_edge_ = false; // consume selected base action BEFORE call
                f.selected_base_entered_ = true;
                try {
                    destroy_native_matrix_model_prefix_storage_00b750c0(
                        f.actual_, f.base_frame_, c.prefix(), f.base_);
                    f.selected_base_returned_ = true;
                } catch (...) {
                    f.secondary_ = std::current_exception();
                    f.phase_ = Phase::failed;
                    std::terminate(); // no Original FH3/counter-policy assertion
                }
            }
            // Lower raw base consumed its own normal/selected actions. End
            // only constructor-issued extents, never replay Native cleanup.
            if (f.base_.node.started) end_extents(f);
            f.phase_ = Phase::failed;
            throw;
        }
        f.destructor_returned_ = true;
    }
    static void destroy(const NativeMatrixStorageContext& c, Frame& f,
        NativeMatrixStorageOwner* receiver = nullptr) {
        require(c, f, Phase::live); // rejected entry never runs another cleanup
        try {
            enter_destroy(c, f, receiver);
            destroy_payload(c, f);
            end_extents(f);
        } catch (...) { failed(f); throw; }
        f.phase_ = Phase::payload_dead;
        healthy(f);
    }
    static void* scalar(const volatile Word& flags,
        const NativeMatrixStorageContext& c, Frame& f, NativeMatrixStorageOwner* receiver = nullptr) {
        require(c, f, Phase::live);
        try {
            enter_destroy(c, f, receiver);
            destroy_payload(c, f);
            const auto low = *reinterpret_cast<const volatile unsigned char*>(&flags);
            f.flags_low_byte_ = low;
            f.flags_read_ = true;
            end_extents(f); // AFTER the last late flags/payload borrow
            if ((low & 1u) != 0) {
                f.pool_return_entered_ = true;
                c.prefix().pool_01090390().return_raw_slot_00b90150(f.actual_);
                f.pool_returned_ = true;
            }
            f.scalar_returned_ = true; // actual result before Source health fence
        } catch (...) { failed(f); throw; }
        f.phase_ = f.pool_returned_ ? Phase::returned : Phase::payload_dead;
        healthy(f);
        return f.actual_; // numeric identity, never dereference after pool return
    }
    static void return_slot(const NativeMatrixStorageContext& c, Frame& f,
        NativeMatrixStorageOwner* receiver = nullptr) {
        require(c, f, Phase::payload_dead);
        custody(c, f, receiver);
        if (f.prefix_extent_.live() || f.bones_live_ || f.pool_return_entered_)
            reject(f, "Matrix physical disposition requires ended extents and unreturned slot");
        try {
            (void)admit_native_matrix_model_allocation(f.actual_, c.prefix());
            f.pool_return_entered_ = true;
            c.prefix().pool_01090390().return_raw_slot_00b90150(f.actual_);
            f.pool_returned_ = true;
        } catch (...) { failed(f); throw; }
        f.phase_ = Phase::returned;
        healthy(f);
    }
};
NativeMatrixStorageFrame::Progress NativeMatrixStorageFrame::progress() const noexcept {
    return {phase_, actual_, captured_bones_data_, poisoned_, allocation_entered_, allocation_returned_, name_captured_,
        constructor_entered_, constructor_returned_, destructor_entered_, destructor_returned_,
        resize_entered_, resize_returned_, free_entered_, free_returned_,
        normal_base_entered_, normal_base_returned_, selected_base_entered_, selected_base_returned_,
        base_.started, base_.complete, factory_return_entered_, factory_return_returned_,
        flags_read_, scalar_returned_, pool_return_entered_, pool_returned_, flags_low_byte_,
        prefix_extent_.live(), bones_live_, creator_.issued, creator_.transferred,
        creator_.release_entered, creator_.release_returned, creator_.decrement_completed,
        creator_.disposition_entered, creator_.receiver_identity, creator_.receiver_ended};
}
NativeMatrixStorageOwner::NativeMatrixStorageOwner(const NativeMatrixStorageContext& c,
    NativeMatrixStorageFrame& f) : context_(c), frame_(f) {
    NativeMatrixStorageAccess::transfer(c, f, *this);
}
NativeMatrixStorageOwner::~NativeMatrixStorageOwner() noexcept {
    if (requires_process_retention()) std::terminate(); // metadata guard, never Native cleanup
    NativeMatrixStorageAccess::end_receiver(*this, frame_);
}
NativeMatrixStorageOwner transfer_native_matrix_creator(const NativeMatrixStorageContext& c,
    NativeMatrixStorageFrame& f) { return NativeMatrixStorageOwner(c, f); } // C++17 elision
void* NativeMatrixStorageOwner::capture_current_raw_scene_170() {
    return NativeMatrixStorageAccess::current_scene(context_, frame_, this);
}
void NativeMatrixStorageOwner::set_geometry_00b75170(const NativeMatrixStorageGeometryContext& services,
    NativeModelStorageGeometryFrame& arguments, NativeModelStorageGeometryAcquired& acquired) {
    NativeMatrixStorageAccess::geometry(context_, frame_, this, services, arguments, acquired);
}
void NativeMatrixStorageOwner::release_creator_00b6f310(NativeNodeTreeRetirementFrame& arguments) {
    NativeMatrixStorageAccess::release_creator(context_, frame_, this, arguments);
    NativeMatrixStorageAccess::settled(*this, frame_);
}
void NativeMatrixStorageOwner::destroy_00b91280() {
    NativeMatrixStorageAccess::destroy(context_, frame_, this);
}
void* NativeMatrixStorageOwner::delete_00b915a0(const volatile Word& flags) {
    void* const identity = NativeMatrixStorageAccess::scalar(flags, context_, frame_, this);
    NativeMatrixStorageAccess::settled(*this, frame_);
    return identity;
}
void NativeMatrixStorageOwner::return_slot() {
    NativeMatrixStorageAccess::return_slot(context_, frame_, this);
    NativeMatrixStorageAccess::settled(*this, frame_);
}
void set_native_matrix_creator_geometry_00b75170(const NativeMatrixStorageContext& c,
    NativeMatrixStorageFrame& f, const NativeMatrixStorageGeometryContext& services,
    NativeModelStorageGeometryFrame& arguments, NativeModelStorageGeometryAcquired& acquired) {
    NativeMatrixStorageAccess::geometry(c, f, nullptr, services, arguments, acquired);
}
void release_native_matrix_creator_00b6f310(const NativeMatrixStorageContext& c,
    NativeMatrixStorageFrame& f, NativeNodeTreeRetirementFrame& arguments) {
    NativeMatrixStorageAccess::release_creator(c, f, nullptr, arguments);
}
void* create_native_matrix_storage_00b931f0(const void* const volatile& home,
    const NativeMatrixStorageContext& c, NativeMatrixStorageFrame& f) {
    return NativeMatrixStorageAccess::create(home, c, f);
}
void* construct_native_matrix_storage_00b91220(const void* name,
    const NativeMatrixStorageContext& c, NativeMatrixStorageFrame& f) {
    return NativeMatrixStorageAccess::construct(name, c, f);
}
void destroy_native_matrix_storage_00b91280(const NativeMatrixStorageContext& c,
    NativeMatrixStorageFrame& f) { NativeMatrixStorageAccess::destroy(c, f); }
void* delete_native_matrix_storage_00b915a0(const volatile Word& flags,
    const NativeMatrixStorageContext& c, NativeMatrixStorageFrame& f) {
    return NativeMatrixStorageAccess::scalar(flags, c, f);
}
void return_native_matrix_storage_slot(const NativeMatrixStorageContext& c,
    NativeMatrixStorageFrame& f) { NativeMatrixStorageAccess::return_slot(c, f); }
} // namespace bsp
