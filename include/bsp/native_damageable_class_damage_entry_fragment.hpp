#pragma once
#include "bsp/native_lua_objects.hpp"

namespace bsp {

// Actual enclosing 0087CA80 storage. S is ESP after E4h locals and four
// saved registers. Unique at S+94h remains live under parent state1.
struct NativeDamageableClassDamageEntryFragmentScratch {
    NativeLuaObjectStorage& live_unique_at_parent_94;
    void* fresh_damage_at_parent_44;   // aligned 14h bytes; stale bytes allowed
    void* fresh_sections_at_parent_80; // aligned 14h bytes; stale bytes allowed
};

// Bounded gates CD4D..CDA8 and matched cleanup tail D1F8..D21C only.
// CD4D is an interior setup instruction, not a native function entrypoint.
// Borrow actual row/owner, stable scratch and live Unique; never copy Lua
// headers, owners, indices or tracked storage. All slots are disjoint, aligned
// at least4 and stable for this owner's entire lifetime, including callbacks.
// Existing protected lookup capacity/index/error-handler contracts apply.
//
// Create this owner BEFORE open, inside the enclosing Unique owner's lifetime.
// Call open exactly once. False has already destroyed the actual live fields
// and reaches D21D. True retains state11 and the actual Damage/Sections objects
// at CDA9: retain this owner through the EXCLUDED iterator/loop continuation
// and all its inner cleanup guards. Only at the matched native cleanup edge
// call close explicitly for normal completion. No success-path auto-close is
// supplied by open, and no iterator, vector or descriptor writes are admitted.
//
// close lowers11->10 before Sections destruction, then10->1 before Damage.
// If explicit Sections cleanup throws, unwind this owner to close remaining
// Damage once. A lookup/predicate failure likewise requires ordinary owner
// unwinding before any further work. Never reopen/replay or mutate the scratch
// binding. The destructor handles propagation; secondary failure follows
// ordinary noexcept termination. This owner never destroys Unique.
//
// Ordinary MSVC Win32 C++ Source and protected Lua error transport only.
// Native register/stack/FH3, SEH/longjmp/double-exception identity, full-parent
// composition, application receiver binding and runtime proof remain held.
class NativeDamageableClassDamageEntryFragment final {
public:
    explicit NativeDamageableClassDamageEntryFragment(
        NativeDamageableClassDamageEntryFragmentScratch& actual_scratch) noexcept;
    ~NativeDamageableClassDamageEntryFragment() noexcept;
    NativeDamageableClassDamageEntryFragment(
        const NativeDamageableClassDamageEntryFragment&) = delete;
    NativeDamageableClassDamageEntryFragment& operator=(
        const NativeDamageableClassDamageEntryFragment&) = delete;
    NativeDamageableClassDamageEntryFragment(
        NativeDamageableClassDamageEntryFragment&&) = delete;
    NativeDamageableClassDamageEntryFragment& operator=(
        NativeDamageableClassDamageEntryFragment&&) = delete;

    bool open(NativeLuaObjectStorage& actual_row);
    void close();

private:
    NativeDamageableClassDamageEntryFragmentScratch& scratch_;
    unsigned state_ = 1;
};

} // namespace bsp
