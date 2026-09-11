#pragma once
#include "bsp/material_lighting.hpp"
#include "bsp/native_render_context.hpp"
#include "bsp/native_string.hpp"
#include <array>
#include <atomic>
#include <cstddef>

namespace bsp {
// SAME110h native material prefix. The allocator's slab ID is a separate DWORD
// at+110 in a114h slot. No field initializer replaces the native preimage.
struct NativeMaterialStorage {
    std::uint32_t vtable_00;
    std::atomic<std::int32_t> references_04;
    std::uint32_t word_08;
    void* source_owner_0c;
    std::array<void*, 9> textures_10;
    std::int16_t texture_count_34;
    std::array<std::byte, 2> preserved_36;
    MaterialLightingValues lighting_38;
    void* effect_7c;
    std::array<void*, 32> parameters_80;
    std::int32_t parameter_count_100;
    std::uint32_t word_104, word_108;
    std::uint8_t lighting_enabled_10c, retain_source_10d;
    std::array<std::byte, 2> preserved_10e;
};

// REQUIRED initialized material pool at00F8D3AC:64 slots/slab, stride114h,
// slab bytes4584h, free-index words+4500, free count+4580, slot slab ID+110.
// These services operate on that SAME shared pool and its real critical section,
// allocation table and allocator-list registration. No CRT fallback is supplied.
class NativeMaterialSlotPool {
public:
    virtual ~NativeMaterialSlotPool() = default;
    virtual void* allocate_00b18200() = 0;
    virtual void return_slot_00b17a80(void* actual_slot) = 0;
};
// REQUIRED distinct initialized parameter pool at00F8D3E4:128 slots/slab,
// stride88h, free-index words+4400, free count+4500, slot slab ID+84.
// Called AFTER name release. Load its current+84 under the real pool lock,
// matching00B193FA..00B19463. Do not free the string again or clear the table.
class NativeMaterialParameterSlots {
public:
    virtual ~NativeMaterialParameterSlots() = default;
    virtual void return_slot_00b193fa_fragment(void* actual_parameter) = 0;
};
struct NativeMaterialDestructionAccess {
    NativeRenderActualOwners& retained_owners;
    NativeStringStorage& parameter_names; // actual00419CC0/BD1510 string pool
    NativeMaterialParameterSlots& parameter_slots;
    NativeMaterialSlotPool& material_slots;
};

// Original ECX fresh material, stack effect/source, EAX same material, RET4.
// At least110h valid aligned storage is required. Pool-backed instances occupy
//114h and retain the live pool ID required later by deleting flags&1.
// The source is a distinct live material; fields/counts and actual retained
// owners must remain valid through each native callback. Unknown/unwritten
// bytes+36,+80..FC,+10E and the pool ID remain untouched.
// Effect must be an actual effect owner with live+04 and writable byte+B4.
// Null effect is supported by the original ordinary constructor. Clone never
// marks effect+B4 and starts with an empty parameter table count.
NativeMaterialStorage* initialize_native_material_00b18900(void* actual_slot,
    void* actual_effect, NativeRenderActualOwners&);
NativeMaterialStorage* clone_native_material_00b18b60(void* actual_slot,
    const NativeMaterialStorage& source, NativeRenderActualOwners&);

// Original ECX material, stack actual owner and low-byte retain flag, RET8.
// Release the old +0C iff +10D is nonzero, clearing it AFTER any terminal
// callback; then store the incoming pointer/exact byte and retain incoming+04
// iff both are nonzero. Equal pointers still release and reacquire. Incoming
// must independently survive old-owner release; no protective retain is added.
void set_native_material_parameter_owner_00b18a40(NativeMaterialStorage&,
    void* actual_owner, std::uint8_t retain_flag, NativeRenderActualOwners&);

// Complete native destruction ordering over actual storage. Reload texture
// end/parameter count after callbacks; clear captured resource slots only after
// release. Source+0C is untouched when exact byte+10D is zero. Parameter slots,
// counts and flags are NOT cleared. Normal and C++ exception exits set CEB130.
// Supported texture counts0..9 and parameter counts<=32 must remain in bounds;
// malformed/reentrant extents are host errors, not native memory corruption.
void destroy_native_material_00b192f0(NativeMaterialStorage&,
    NativeMaterialDestructionAccess&);
NativeMaterialStorage* delete_native_material_00b194b0(NativeMaterialStorage*,
    NativeMaterialDestructionAccess&, std::uint32_t flags);

//00B18780 simply replaces ECX with global00F8D3AC then jumps to00B18200.
// The caller's apparent sizeof110h ECX input is ignored by the original thunk.
void* allocate_native_material_slot_00b18780(NativeMaterialSlotPool&);

class NativeMaterialReference;
struct NativeMaterialCompanionDisposal {
    void* context;
    // Unregister/delete this SAME companion after native destruction and pool
    // return. No actual storage or companion access follows this callback.
    void (*retire)(void*, NativeMaterialReference&) noexcept;
};
// Canonical lifetime only, borrowing actual+04 without initialization/retention.
// Caller registers it in the SAME NativeRenderActualOwners as section/model
// references. This class adds no registry, material copy, shader or extra count.
// Its existence does NOT provide a MaterialCloneState projection or rendering.
class NativeMaterialReference final : public RenderCommandReference {
public:
    NativeMaterialReference(NativeMaterialStorage&, NativeMaterialDestructionAccess&,
        const volatile std::uint32_t* actual_vtable_00d5e520, NativeMaterialCompanionDisposal);
    ~NativeMaterialReference() override;
    NativeMaterialStorage& storage() noexcept { return storage_; }
    void release_zero_references() noexcept override;
private:
    enum class Phase { bound, destroying, retired };
    NativeMaterialStorage& storage_;
    NativeMaterialDestructionAccess& access_;
    const volatile std::uint32_t* vtable_;
    NativeMaterialCompanionDisposal disposal_;
    Phase phase_{Phase::bound};
    void require_current_profile() const noexcept;
};
} // namespace bsp
