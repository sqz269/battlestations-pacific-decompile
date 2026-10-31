#include "bsp/native_plain_node_storage.hpp"
#include "bsp/native_node_pool_allocation.hpp"
#include <cstddef>
#include <new>
#include <stdexcept>
#include <type_traits>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Raw Plain storage requires MSVC Win32.
#endif
namespace bsp {
namespace {
using Word = std::uint32_t;
static_assert(sizeof(void*) == 4 && sizeof(std::uintptr_t) == 4);
static_assert(sizeof(NativeNodeStorage) == 0x174);
static_assert(std::is_trivially_destructible_v<NativeNodeStorage>);
static_assert(std::is_trivially_destructible_v<NativeString>);
Word bits(const void* p) noexcept { return reinterpret_cast<Word>(p); }
void* at(void* p, Word n) noexcept { return reinterpret_cast<void*>(bits(p) + n); }
Word read_word(const volatile void* base, Word byte_offset = 0) noexcept {
    Word value;
    __asm {
        mov eax, base
        mov edx, byte_offset
        mov eax, dword ptr [eax + edx]
        mov value, eax
    }
    return value;
}
Word read_half(const void* base, Word byte_offset) noexcept {
    Word value;
    __asm {
        mov eax, base
        mov edx, byte_offset
        movzx eax, word ptr [eax + edx]
        mov value, eax
    }
    return value;
}
std::uint8_t read_byte(const volatile void* base) noexcept {
    std::uint8_t value;
    __asm {
        mov eax, base
        mov al, byte ptr [eax]
        mov value, al
    }
    return value;
}
struct Region { const void* address; std::size_t bytes; };
bool disjoint(Region a, Region b) noexcept {
    const auto x = static_cast<std::uint64_t>(bits(a.address));
    const auto y = static_cast<std::uint64_t>(bits(b.address));
    return x + a.bytes <= y || y + b.bytes <= x;
}
void require_live_slot(void* slot, void* pool) {
    const Word index = read_word(slot, 0x174);
    const Word table = read_word(pool, 0x28);
    const Word tables = read_word(pool, 0x2c);
    if (!table || index >= tables)
        throw std::invalid_argument("Plain178 slot requires its CURRENT pool table/index174");
    void* const slab = reinterpret_cast<void*>(read_word(reinterpret_cast<void*>(table), index * 4u));
    const Word delta = bits(slot) - bits(slab);
    if (!slab || delta >= 0x2f00u || delta % 0x178u != 0u)
        throw std::invalid_argument("Plain slot is not an exact cell in its CURRENT slab");
    const Word ordinal = delta / 0x178u;
    const Word count = read_half(slab, 0x2f40);
    if (count > 32u) throw std::invalid_argument("Plain slab free-count is invalid");
    Word seen = 0;
    for (Word i = 0; i < count; ++i) {
        const Word free_index = read_half(slab, 0x2f00u + i * 2u);
        if (free_index >= 32u || (seen & (1u << free_index)) != 0u || free_index == ordinal)
            throw std::invalid_argument("Plain slot is free or its free-index metadata is invalid");
        seen |= 1u << free_index;
    }
}
} // namespace
NativePlainNodeStorageContext::NativePlainNodeStorageContext(
    NativeNodeBaseDestructionContext& node, void* pool, const NativeNodeRawConstants& constants)
    : node_(node), pool_(pool), constants_(constants) {
    if (!pool_) throw std::invalid_argument("Plain context requires its genuine initialized0108FF58");
    if (static_cast<NativeRenderActualOwners*>(&node_.scenes.owners) != &node_.trees.owners ||
        &node_.scenes.decrement_00ce2220 != &node_.trees.decrement_00ce2220)
        throw std::invalid_argument("Plain context requires SAME actual owners/decrement cell");
}
struct NativePlainNodeStorageAccess {
    using Frame = NativePlainNodeStorageFrame;
    using Phase = Frame::Phase;
    using Creator = Frame::Creator;
    static void reject(Frame& f, const char* message) {
        f.poisoned_ = true;
        try { throw std::logic_error(message); }
        catch (...) {
            if (!f.primary_) f.primary_ = std::current_exception();
            throw;
        }
    }
    static void require(const NativePlainNodeStorageContext& c, Frame& f, Phase phase) {
        if (f.poisoned_ || f.phase_ != phase || (f.context_ && f.context_ != &c))
            reject(f, "Plain context/operation cannot replay or reenter");
    }
    static void fail(Frame& f) noexcept {
        if (!f.primary_) f.primary_ = std::current_exception();
        if (f.creator_ == Creator::owed) f.creator_ = Creator::retained_failure;
        f.phase_ = Phase::failed;
    }
    static void healthy(Frame& f) {
        if (f.poisoned_) {
            fail(f);
            reject(f, "Plain owning invocation was poisoned");
        }
    }
    static void backing(const NativePlainNodeStorageContext& c, Frame& f) {
        const Region payload{f.actual_, native_node_pool_slot_bytes};
        const Region records[] = {{&f, sizeof f}, {&c, sizeof c},
            {&c.node(), sizeof(c.node())}, {&c.node().scenes, sizeof(c.node().scenes)},
            {&c.node().trees, sizeof(c.node().trees)}, {&c.node().strings, sizeof(c.node().strings)},
            {&c.constants(), sizeof(c.constants())}, {c.pool_0108ff58(), 0x38}};
        for (const auto record : records)
            if (!disjoint(payload, record))
                throw std::invalid_argument("Plain full178 slot overlaps retained metadata/pool");
        for (std::size_t i = 1; i != sizeof(records) / sizeof(records[0]); ++i)
            if (!disjoint(records[0], records[i]))
                throw std::invalid_argument("Plain frame overlaps its retained context");
    }
    static void end_type(Frame& f) noexcept {
        if (f.node_live_) {
            f.node_live_ = false; // consume before actual trivial C++ extent end
            f.node_->~NativeNodeStorage();
            f.type_ended_ = true;
        }
    }
    static NativeNodeStorage* create(const volatile void* home,
        const NativePlainNodeStorageContext& c, Frame& f) {
        require(c, f, Phase::fresh); // rejected invocation owns no selected edge
        f.context_ = &c;
        f.phase_ = Phase::creating;
        f.allocation_entered_ = true;
        try {
            f.actual_ = allocate_native_node_00b6ed70(c.pool_0108ff58());
            f.allocation_returned_ = true;
            if (f.actual_) {
                f.factory_edge_ = true; // real captured allocation/state0
                require_live_slot(f.actual_, c.pool_0108ff58());
                backing(c, f);
                const Word name = read_word(home); // nonnull ONLY; one late4B load
                f.name_sampled_ = true;
                f.constructor_entered_ = true;
                f.node_ = &construct_native_node_00b6f5a0(f.actual_, native_node_pool_slot_bytes,
                    reinterpret_cast<void*>(name), c.node().strings, c.constants());
                f.constructor_returned_ = true;
                f.node_live_ = true; // only this actual normal return issues birth
                f.creator_ = Creator::owed;
                f.factory_edge_ = false; // disarm BEFORE Source health/publication
            }
        } catch (...) {
            f.primary_ = std::current_exception();
            if (f.factory_edge_) {
                f.factory_edge_ = false; // captured-slot action consumed first
                f.factory_return_entered_ = true;
                try {
                    return_native_node_00b6e670(f.actual_, c.pool_0108ff58());
                    f.factory_return_returned_ = true;
                } catch (...) {
                    f.secondary_ = std::current_exception();
                    fail(f);
                    throw; // existing Source composition; NOT Native FH3 secondary policy
                }
            }
            fail(f);
            throw;
        }
        f.phase_ = f.actual_ ? Phase::live : Phase::null_result;
        healthy(f); // completed birth is never returned as failed construction
        return f.node_;
    }
    static void* scalar(const volatile void* flags,
        const NativePlainNodeStorageContext& c, Frame& f) {
        require(c, f, Phase::live); // rejected inner entry has no cleanup scope
        if (!f.constructor_returned_ || !f.node_live_)
            reject(f, "Plain scalar requires its genuine constructor-issued extent");
        f.phase_ = Phase::destroying;
        try {
            require_live_slot(f.actual_, c.pool_0108ff58());
            backing(c, f);
            f.destructor_entered_ = true;
            destroy_native_raw_node_base_00b6f440(f.actual_, f.destruction_frame_,
                c.node(), f.destruction_);
            f.destructor_returned_ = true;
            const std::uint8_t low = read_byte(flags); // CURRENT after B6F440
            f.flags_low_byte_ = low;
            f.flags_read_ = true;
            end_type(f); // AFTER last flags borrow; index174 remains live
            f.creator_ = Creator::retired_by_direct_scalar; // not a Native count decrement
            if ((low & 1u) != 0u) {
                f.pool_return_entered_ = true;
                return_native_node_pool_slot_00b6e490(c.pool_0108ff58(), f.actual_);
                f.pool_returned_ = true;
            }
            f.scalar_returned_ = true;
        } catch (...) {
            // Lower raw body has consumed its selected actions. No retry/return.
            if (f.destruction_.started) end_type(f);
            fail(f);
            throw;
        }
        f.phase_ = f.pool_returned_ ? Phase::returned : Phase::payload_dead;
        healthy(f);
        return f.actual_; // identity only after possible physical return
    }
    static void return_slot(const NativePlainNodeStorageContext& c, Frame& f) {
        require(c, f, Phase::payload_dead);
        if (f.node_live_ || f.pool_return_entered_)
            reject(f, "Plain explicit disposition requires dead type and unreturned slot");
        try {
            require_live_slot(f.actual_, c.pool_0108ff58()); // only live index174/slab metadata
            f.pool_return_entered_ = true;
            return_native_node_pool_slot_00b6e490(c.pool_0108ff58(), f.actual_);
            f.pool_returned_ = true;
        } catch (...) { fail(f); throw; }
        f.phase_ = Phase::returned;
        healthy(f);
    }
};
NativePlainNodeStorageFrame::Progress NativePlainNodeStorageFrame::progress() const noexcept {
    return {phase_, creator_, actual_, poisoned_, node_live_, allocation_entered_, allocation_returned_,
        name_sampled_, constructor_entered_, constructor_returned_, factory_return_entered_,
        factory_return_returned_, destructor_entered_, destructor_returned_, destruction_.started,
        destruction_.complete, flags_read_, type_ended_, pool_return_entered_, pool_returned_,
        scalar_returned_, flags_low_byte_};
}
NativeNodeStorage* create_native_plain_node_storage_00b866c0(const volatile void* home,
    const NativePlainNodeStorageContext& c, NativePlainNodeStorageFrame& f) {
    return NativePlainNodeStorageAccess::create(home, c, f);
}
void* delete_native_plain_node_storage_00b6f8d0(const volatile void* flags,
    const NativePlainNodeStorageContext& c, NativePlainNodeStorageFrame& f) {
    return NativePlainNodeStorageAccess::scalar(flags, c, f);
}
void return_native_plain_node_storage_slot(const NativePlainNodeStorageContext& c,
    NativePlainNodeStorageFrame& f) { NativePlainNodeStorageAccess::return_slot(c, f); }
} // namespace bsp
