#pragma once
#include "bsp/native_unit_scene_initialization.hpp"
#include "bsp/native_lua_objects.hpp"

namespace bsp {
// Actual checked list at F87194: opaque +0, sentinel +4, count +8. These
// bindings borrow existing list/node fields; they allocate no alternative list.
struct NativeUnitActivationListView {
    void* identity;
    void* const& sentinel_04;
    std::uint32_t& count_08;
};
struct NativeUnitActivationListNodeView {
    void*& next_00;
    void*& previous_04;
    void* const& unit_08;
};
struct NativeUnitActivationView {
    NativeUnitObserverAlias unit;
    std::uint32_t& side_54;
    void*& property_holder_c0; // same actual cell as scene/health-parts views
    std::uint32_t* roles_188; // nine actual consecutive DWORDs
    std::uint32_t& word_28c;
    std::uint32_t& tick_294;
    float& marker_304;
};
struct NativeUnitActivationSavedView {
    const std::uint32_t& word_40;
    const std::uint32_t* roles_64; // nine actual consecutive DWORDs
    const std::uint32_t& side_b0;
    const float& marker_108;
    const std::uint32_t& role_10c;
};
struct NativeUnitActivationGlobals {
    NativeUnitActivationListView list_00f87194;
    const std::uint32_t& tick_00f876b0;
};
// Only native stack scratch, not a constructed reader or production owner.
// Provider 4425C0 must construct all reached fields and consume the Lua root.
struct NativeUnitActivationReaderScratch { alignas(4) std::byte bytes[0x14]; };

class NativeUnitActivationBindings {
public:
    virtual ~NativeUnitActivationBindings() = default;
    // Pure nonthrowing actual-storage bindings. No callback, allocation,
    // default, owner substitution, mutation or FP-stack/environment changes.
    virtual NativeUnitActivationListNodeView list_node(void*) noexcept = 0;
    virtual NativeUnitScenePropertyHolderView property_holder(void*) noexcept = 0;
    virtual NativeUnitActivationSavedView saved_state(void*) noexcept = 0;
    virtual NativeUnitScenePropertyView property_record(void*) noexcept = 0;
    virtual const volatile std::uint32_t* primary_table(void*) noexcept = 0;

    // Required COMPLETE providers; no default/stub or replacement container.
    // 4C2220: ECX list (unused), stack next, previous, &unit; EAX node; RET0C.
    virtual void* call_004c2220(NativeUnitActivationListView, void* next,
        void* previous, void* const* unit) = 0;
    // 9277E0 tail-jumps 923840: conditional parent race inheritance, +5C/+BD=1.
    virtual void call_009277e0(const NativeUnitObserverAlias&) = 0;
    // 9238A0: constructs actual savedata._entities[index] tracked Lua object;
    // index is reread from CURRENT unit+holderC0+8 inside this operation.
    virtual void call_009238a0(void* unit, NativeLuaObjectStorage& fresh) = 0;
    // Takes the SAME root object, the native by-value stack argument identity;
    // owns its destruction and full constructor-internal cleanup. Native RET14.
    virtual void call_004425c0(NativeUnitActivationReaderScratch& fresh,
        NativeLuaObjectStorage& consumed_stack_argument) = 0;
    virtual void call_00441a20(NativeUnitActivationReaderScratch&) = 0;
    virtual std::uint8_t call_0048e9f0(void* bag, const char* key) = 0;
    virtual void* call_008f2260(void* bag, const char* key) = 0;
    // Capture current table/entry at each evidenced site. Tokens are resolved
    // by the owning runtime, never cast into native process code in production.
    virtual void call_unit_144(std::uint32_t entry, void* unit, std::uint32_t role) = 0;
    virtual void call_unit_f4(std::uint32_t entry, void* unit, float value) = 0;
};

// Complete normal 0077F0E0..0077F24C caller. Original ECX unit, saves
// EBX/ESI/EDI, bare RET. Stable actual reached fields/tables are required.
// Explicit volatile accesses preserve repeated reads and publication ordering;
// x87 FLD/FSTP and SSE conversion preserve the native floating-point steps.
// No native FH3/SEH, drop-in ABI, owning runtime or gameplay claim.
void activate_native_unit_0077f0e0(NativeUnitActivationView,
    NativeUnitActivationGlobals, NativeUnitActivationBindings&);
} // namespace bsp
