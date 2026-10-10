#pragma once
#include "bsp/native_damageable_class_damage_entry_fragment.hpp"

namespace bsp {

// Actual enclosing 0087CA80 storage; S is ESP after E4h locals/four saves.
// The Damage entry owner has returned true and still owns Damage S+44h and
// Sections S+80h under state11; the enclosing caller still owns Unique S+94h.
struct NativeDamageableClassSectionIteratorSetupFragmentScratch {
    NativeDamageableClassDamageEntryFragment& live_damage_entry_owner;
    NativeLuaObjectStorage& live_sections_at_parent_80;
    void* fresh_key_at_parent_58;   // aligned 14h bytes; stale bytes allowed
    void* fresh_value_at_parent_2c; // aligned 14h bytes; stale bytes allowed
};

// Setup CDA9..CDFF and pair cleanup D1CF..D1F0 only. Neither is a native
// entrypoint. CE00 loop body, D1F1 register reload and outer D1F8 cleanup are
// excluded. Borrow stable, disjoint actual slots and scratch binding; never
// copy Lua objects, owners, indices or tracked storage. Alignment is at least4.
// All existing Lua owner/stack/reference-capacity/error-handler contracts apply.
// An inherited handler must remain below every removed/consumed slot, including
// surrounding cleanup. Use the same protected ordinary C++ reader boundary.
//
// Construct this fixed owner BEFORE its one permitted open, inside the already
// successful Damage entry owner's lifetime. Construct actual key first (state12),
// then value (state13), and run genuine protected first iteration on Sections.
// False means empty: the pair is already closed to state11. True retains state13
// and the actual key/value through the EXCLUDED loop and every inner guard.
// At the matched normal tail call close explicitly, then let the enclosing
// Damage entry owner close Sections/Damage at its own matching tail.
//
// close lowers13->12 BEFORE value destruction, then12->11 BEFORE key destruction.
// If explicit value cleanup throws, owner unwinding destroys the remaining key
// once. First-iteration failure likewise requires ordinary owner unwinding before
// further work. This owner never destroys Sections, Damage or Unique. No reopen,
// replay, callback reentry or scratch-binding mutation is allowed. Its noexcept
// destructor handles propagation; secondary failure follows ordinary termination.
//
// Ordinary MSVC Win32 Source only. Native ABI/FH3, SEH/longjmp/fault identity,
// full-loop/parent composition, actual receiver binding and runtime proof remain
// held. No next-iteration, vector, section-row, effect or category work is supplied.
class NativeDamageableClassSectionIteratorSetupFragment final {
public:
    explicit NativeDamageableClassSectionIteratorSetupFragment(
        NativeDamageableClassSectionIteratorSetupFragmentScratch& actual_scratch) noexcept;
    ~NativeDamageableClassSectionIteratorSetupFragment() noexcept;
    NativeDamageableClassSectionIteratorSetupFragment(
        const NativeDamageableClassSectionIteratorSetupFragment&) = delete;
    NativeDamageableClassSectionIteratorSetupFragment& operator=(
        const NativeDamageableClassSectionIteratorSetupFragment&) = delete;
    NativeDamageableClassSectionIteratorSetupFragment(
        NativeDamageableClassSectionIteratorSetupFragment&&) = delete;
    NativeDamageableClassSectionIteratorSetupFragment& operator=(
        NativeDamageableClassSectionIteratorSetupFragment&&) = delete;

    bool open();
    void close();

private:
    NativeDamageableClassSectionIteratorSetupFragmentScratch& scratch_;
    unsigned state_ = 11;
};

} // namespace bsp
