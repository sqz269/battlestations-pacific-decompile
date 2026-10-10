#pragma once
#include "bsp/native_damageable_class_section_iterator_setup_fragment.hpp"
#include "bsp/native_damageable_section_vector.hpp"

namespace bsp {

// Borrow the successful actual iterator owner and its unchanged scratch. It
// still holds key S+58h/value S+2Ch under parent state13; S is ESP after the
// native parent's E4h locals and four saves. Outer Lua ownership is unchanged.
struct NativeDamageableClassSectionRowBeginFragmentScratch {
    NativeDamageableClassSectionIteratorSetupFragment& live_iterator_owner;
    NativeDamageableClassSectionIteratorSetupFragmentScratch& actual_iterator_scratch;
    void* fresh_section_at_parent_a8; // actual aligned, writable 30h bytes
};

struct NativeDamageableClassSectionRowBeginFragmentAccess {
    const NativeDamageableSectionVectorAccess& actual_vector_access;
    const volatile std::uint32_t& actual_feature_word_0109eea4;
    const void* actual_default_float_cell_00ce38b8; // actual stable 41200000h cell
};

// Only 0087CE00..0087CE87: initialize actual temporary, append through actual
// descriptor+18h header, and finish its matching normal cleanup. CE88 row
// selection and every later field/iteration are excluded. No row is returned.
//
// All borrowed identities/bindings remain stable for this fixed owner's life.
// The actual descriptor header is 10h bytes with +4 begin/+8 end/+C capacity;
// its +0 word is preserved. The 48-byte temporary is disjoint from the actual
// descriptor, live Lua slots, this owner, bindings and provider frames/arguments.
// Require alignment at least4, writable nonwrapping extents and DF=0. Supply
// genuine stable table, returning invalid-parameter service/context, canonical
// CRT cell alias and CE38B8 cell containing the retained float32 10.0 bits.
// There is no fallback cell, invented table/service, surrogate row or container.
// Existing vector allocation, x87-copy, ownership and exception contracts apply.
//
// Construct BEFORE the one permitted run, inside the live state13 iterator
// owner's lifetime. It stays armed14 only across genuine append. Normal cleanup
// captures owner+24/nullness before lowering14->13 and writing the actual table;
// decrement/current-vslot0 then clear the original slot only after normal return.
// A throwing normal callback must not retry temporary cleanup. Append failure
// uses genuine878EF0 on active14 unwind; it does not roll back the vector.
// No run replay, callback reentry or scratch/access-binding mutation is allowed.
// The noexcept destructor qualifies secondary failure as ordinary termination.
// Enclosing pair, Damage, Sections and Unique owners remain the caller's duty.
//
// Ordinary MSVC Win32 Source only: no original entry ABI/FH3, SEH/longjmp/fault
// identity, application receiver binding, full-reader or runtime claim.
class NativeDamageableClassSectionRowBeginFragment final {
public:
    NativeDamageableClassSectionRowBeginFragment(void* actual_descriptor,
        NativeDamageableClassSectionRowBeginFragmentScratch& actual_scratch,
        const NativeDamageableClassSectionRowBeginFragmentAccess& actual_access) noexcept;
    ~NativeDamageableClassSectionRowBeginFragment() noexcept;
    NativeDamageableClassSectionRowBeginFragment(
        const NativeDamageableClassSectionRowBeginFragment&) = delete;
    NativeDamageableClassSectionRowBeginFragment& operator=(
        const NativeDamageableClassSectionRowBeginFragment&) = delete;
    NativeDamageableClassSectionRowBeginFragment(
        NativeDamageableClassSectionRowBeginFragment&&) = delete;
    NativeDamageableClassSectionRowBeginFragment& operator=(
        NativeDamageableClassSectionRowBeginFragment&&) = delete;

    void run();

private:
    void* const descriptor_;
    NativeDamageableClassSectionRowBeginFragmentScratch& scratch_;
    const NativeDamageableClassSectionRowBeginFragmentAccess& access_;
    unsigned state_ = 13;
};

} // namespace bsp
