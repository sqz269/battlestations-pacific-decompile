#include "bsp/native_mesh_storage_lifetime.hpp"
#include <array>
#include <memory>
#include <stdexcept>
#include <type_traits>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native Mesh storage lifetime targets MSVC Win32.
#endif

namespace bsp {
static_assert(sizeof(void*) == 4 && sizeof(std::uintptr_t) == 4);
static_assert(sizeof(NativeMeshStorage) == 0xbc);
static_assert(alignof(NativeMeshStorage) == 4);
static_assert(offsetof(NativeMeshStorage, references_04) == 4);
static_assert(NativeMeshPool::object_bytes == 0xbc);
static_assert(NativeMeshPool::slot_chunk_index_offset == 0xbc);
static_assert(NativeMeshPool::slot_bytes == 0xc0);
static_assert(std::is_trivially_destructible_v<NativeMeshStorage>);
static_assert(std::is_nothrow_destructible_v<NativeMeshStorage>);

namespace {
struct Region { const volatile void* base; std::size_t bytes; };
std::uintptr_t address(const volatile void* p) noexcept {
    return reinterpret_cast<std::uintptr_t>(p);
}
bool overlaps(Region a, Region b) noexcept {
    const auto x = static_cast<std::uint64_t>(address(a.base));
    const auto y = static_cast<std::uint64_t>(address(b.base));
    return x < y + b.bytes && y < x + a.bytes;
}
} // namespace

class NativeMeshStorageAccess final {
    using Frame = NativeMeshStorageFrame;
    using Context = NativeMeshStorageContext;
    using Phase = Frame::Phase;
    static std::array<Region, 6> sources(const Context& c) noexcept {
        auto& e = c.environment_;
        return {{{&e, sizeof(e)}, {&e.pool_0108fff8, sizeof(e.pool_0108fff8)},
            {&e.pool_0108fff8.storage(), sizeof(NativeMeshPoolStorage)},
            {&c.constants_.minimum_00ce4adc, 4},
            {&c.constants_.maximum_00ce4970, 4}, {&c.constants_.lod_00d7a24c, 4}}};
    }
    static void metadata_disjoint(const Frame& f, Region r) {
        if (overlaps({&f, sizeof(f)}, r) || overlaps({&f.context_, sizeof(Context)}, r))
            throw std::invalid_argument("Mesh frame/context backing overlaps a borrowed region");
    }
    static void allocation_backing(const Frame& f) {
        if (!f.actual_) return;
        if (f.actual_ % alignof(NativeMeshStorage) != 0 ||
            static_cast<std::uint64_t>(f.actual_) + NativeMeshPool::slot_bytes > (1ull << 32))
            throw std::invalid_argument("Mesh allocation has invalid C0h backing");
        const Region slot{reinterpret_cast<void*>(f.actual_), NativeMeshPool::slot_bytes};
        metadata_disjoint(f, slot);
        for (const auto r : sources(f.context_))
            if (overlaps(slot, r))
                throw std::invalid_argument("Mesh allocation overlaps its live environment/constant backing");
    }
    static void healthy(Frame& f) {
        if (f.poisoned_) {
            f.failed_ = true;
            f.phase_ = Phase::failed; // fail closed BEFORE constructing the exception
            throw std::logic_error("Mesh lifecycle invocation was reentered or reused");
        }
    }
    static void end_type(Frame& f) noexcept {
        // Entry already proved this genuine live type. No payload access follows
        // the explicit no-store destructor; BC..BF is a separate pool region.
        auto* const live = f.live_payload_;
        f.type_end_entered_ = true;
        live->~NativeMeshStorage();
        f.live_payload_ = nullptr;
        f.cpp_payload_live_ = false;
        f.type_ended_ = true;
        f.normal_payload_dead_ = true;
    }
public:
    static void validate_context(const Context& c) {
        for (const auto r : sources(c))
            if (overlaps({&c, sizeof(c)}, r))
                throw std::invalid_argument("Mesh context must have disjoint Source backing");
    }
    static void validate_frame(const Frame& f) {
        if (overlaps({&f, sizeof(f)}, {&f.context_, sizeof(Context)}))
            throw std::invalid_argument("Mesh frame and context must be disjoint");
        for (const auto r : sources(f.context_)) metadata_disjoint(f, r);
    }
    static void reject(Frame& f, const char* message) {
        f.poisoned_ = true; // do not reset an outer invocation's active/fact state
        throw std::logic_error(message);
    }
    static void* allocate(Frame& f) {
        f.begin_allocation(false); // freshness rejection is outside the armed catch
        try {
            f.enter_pool_allocation(f.context_.environment_.pool_0108fff8);
            void* const actual = f.receiver_->allocate_slot_00b73a10();
            return f.allocation_normal_return(actual);
        } catch (...) { f.record_failure(); throw; }
    }
    static NativeMeshStorage* construct(Frame& f) {
        if (f.active_ || f.poisoned_ || f.failed_ ||
            (f.phase_ != Phase::allocated && f.phase_ != Phase::null_allocation))
            reject(f, "Mesh construction requires its same fresh normal allocation");
        f.active_ = true;
        try {
            if (!f.actual_) {
                f.null_construction_skipped_ = true;
                f.phase_ = Phase::returned; // no object, count or physical obligation
                f.active_ = false;
                healthy(f);
                return nullptr;
            }
            allocation_backing(f);
            f.phase_ = Phase::constructing;
            f.constructor_entered_ = true;
            auto* const result = construct_native_mesh_00b73d70(
                reinterpret_cast<void*>(f.actual_), f.context_.constants_);
            f.constructor_returned_ = true; // actual callee returned, before Source fences
            if (address(result) != f.actual_)
                throw std::logic_error("Mesh constructor returned a different allocation");
            f.live_payload_ = result;
            f.cpp_payload_live_ = true;
            f.count_ = address(std::addressof(result->references_04)); // ADDRESS only, while live
            if (f.count_ != f.actual_ + 4u)
                throw std::logic_error("Mesh constructor count is not its actual +4 subobject");
            f.phase_ = Phase::live;
            f.active_ = false;
            healthy(f);
            return result;
        } catch (...) { f.record_failure(); throw; }
    }
    static void* scalar(const volatile std::uint32_t& flags, Frame& f) {
        if (f.active_ || f.poisoned_ || f.failed_ || f.phase_ != Phase::live ||
            !f.constructor_returned_ || !f.cpp_payload_live_ || !f.live_payload_)
            reject(f, "Mesh scalar requires its genuine live constructor-issued payload");
        f.active_ = true;
        try {
            allocation_backing(f);
            metadata_disjoint(f, {std::addressof(flags), sizeof(flags)});
            if (address(f.live_payload_) != f.actual_ ||
                address(std::addressof(f.live_payload_->references_04)) != f.count_)
                throw std::logic_error("Mesh scalar lost its live actual/count identity");
            f.flags_home_ = address(std::addressof(flags)); // no flags value sample
            f.flags_home_captured_ = true;
            f.phase_ = Phase::destroying;
            f.scalar_entered_ = true;
            f.destructor_entered_ = true;
            // Reuse the WHOLE existing ordinary C++ body, including its actual
            // noexcept leaves/RAII termination effects. No Native unwind claim.
            destroy_native_mesh_00b73e60(*f.live_payload_, f.context_.environment_);
            f.destructor_returned_ = true;
            const auto low = *reinterpret_cast<const volatile unsigned char*>(std::addressof(flags));
            f.flags_low_byte_ = low;
            f.flags_read_ = true; // final borrowed HOME access, even if it aliases BC
            end_type(f);
            if ((low & 1u) != 0u) {
                f.pool_return_entered_ = true;
                // Existing noexcept provider. Only its actual normal return is
                // recorded; terminate/SEH is not made catchable by this wrapper.
                f.receiver_->return_slot_00b72da0(reinterpret_cast<void*>(f.actual_));
                f.pool_returned_ = true;
            }
            f.scalar_body_completed_ = true; // whole operation schedule before Source health fence
            f.phase_ = f.pool_returned_ ? Phase::returned : Phase::payload_dead;
            f.active_ = false;
            healthy(f);
            return reinterpret_cast<void*>(f.actual_); // opaque numeric identity only
        } catch (...) { f.record_failure(); throw; }
    }
    friend class NativeMeshStorageFrame;
};

NativeMeshStorageContext::NativeMeshStorageContext(NativeMeshEnvironment& e, NativeMeshConstants c)
    : environment_(e), constants_(c) { NativeMeshStorageAccess::validate_context(*this); }
NativeMeshStorageFrame::NativeMeshStorageFrame(NativeMeshStorageContext& c) : context_(c) {
    NativeMeshStorageAccess::validate_frame(*this);
}
void NativeMeshStorageFrame::begin_allocation(bool from_static) {
    if (active_ || poisoned_ || failed_ || phase_ != Phase::fresh)
        NativeMeshStorageAccess::reject(*this, "Mesh allocation cannot adopt/replay an existing frame");
    active_ = true;
    static_wrapper_entered_ = from_static;
    phase_ = Phase::selecting_receiver;
}
void NativeMeshStorageFrame::enter_pool_allocation(NativeMeshPool& receiver) {
    receiver_ = &receiver;
    receiver_selected_ = true; // actual selected object, not yet an allocation call
    if (&receiver != &context_.environment_.pool_0108fff8)
        throw std::invalid_argument("Mesh allocation receiver differs from its retained environment");
    NativeMeshStorageAccess::validate_frame(*this);
    phase_ = Phase::allocating;
    allocation_entered_ = true; // immediately before that real receiver's allocate call
}
void* NativeMeshStorageFrame::allocation_normal_return(void* actual) {
    allocation_returned_ = true;
    actual_ = address(actual); // actual normal return, before any Source validation
    NativeMeshStorageAccess::allocation_backing(*this);
    phase_ = actual_ ? Phase::allocated : Phase::null_allocation;
    active_ = false;
    NativeMeshStorageAccess::healthy(*this);
    return actual;
}
void NativeMeshStorageFrame::record_failure() noexcept {
    failed_ = true;
    phase_ = Phase::failed;
    active_ = false;
    if (!primary_) primary_ = std::current_exception();
}
bool NativeMeshStorageFrame::requires_process_retention() const noexcept {
    if (active_ || (allocation_entered_ && !allocation_returned_)) return true;
    // Pure pre-allocation failure and a normal NULL allocation own no slot/type.
    if (!actual_) return false;
    return !pool_returned_ || cpp_payload_live_ || failed_ || poisoned_;
}
NativeMeshStorageFrame::Progress NativeMeshStorageFrame::progress() const noexcept {
    return {phase_, actual_, count_, flags_home_, receiver_, active_, poisoned_, failed_,
        static_wrapper_entered_, receiver_selected_, allocation_entered_, allocation_returned_,
        allocation_entered_ && !allocation_returned_, constructor_entered_, constructor_returned_,
        constructor_entered_ && !constructor_returned_, null_construction_skipped_, cpp_payload_live_,
        scalar_entered_, destructor_entered_, destructor_returned_, flags_home_captured_, flags_read_,
        type_end_entered_, type_ended_, normal_payload_dead_, pool_return_entered_, pool_returned_,
        pool_return_entered_ && !pool_returned_, scalar_body_completed_, flags_low_byte_};
}
void* allocate_native_mesh_storage_slot(NativeMeshStorageFrame& f) {
    return NativeMeshStorageAccess::allocate(f);
}
NativeMeshStorage* construct_native_mesh_storage_00b73d70(NativeMeshStorageFrame& f) {
    return NativeMeshStorageAccess::construct(f);
}
void* delete_native_mesh_storage_00b74280(const volatile std::uint32_t& flags, NativeMeshStorageFrame& f) {
    return NativeMeshStorageAccess::scalar(flags, f);
}
} // namespace bsp
