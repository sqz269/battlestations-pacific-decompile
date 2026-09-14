#pragma once
#include "bsp/native_render_pointer_arrays.hpp"
#include "bsp/native_string.hpp"

namespace bsp {
struct NativeNodeStorage;

// Borrowed cells in the EXISTING fresh 1AC-byte collision/model allocation.
// No owner, padded native-layout cast, proxy initialization or shadow vector.
// The unit is the same canonical identity carried by NativeUnitObserverAlias;
// null is allowed. The selected set is the ownership transferred to +160.
struct NativeUnitPartListConstructionView {
    void* identity; // native +178, +188 or +194; proxy word is untouched
    void*& head_04;
    std::uint32_t& count_08;
};
struct NativeUnitPartConstructionView {
    void* identity;
    std::uint32_t& primary_table_00;
    void*& owner_alias_4c;
    void*& selected_set_160;
    void*& owner_164;
    void* groups_identity_168;
    void*& groups_begin_16c;
    void*& groups_end_170;
    void*& groups_capacity_174;
    NativeUnitPartListConstructionView list_178;
    std::uint8_t& attached_184;
    NativeUnitPartListConstructionView list_188;
    NativeUnitPartListConstructionView list_194;
    NativeRenderPointerArrayStorage& entries_1a0;
};

class NativeUnitPartConstructionBindings {
public:
    virtual ~NativeUnitPartConstructionBindings() = default;
    // Pure nonthrowing access to current actual backing, no caching or callback.
    virtual const volatile std::uint32_t* primary_table(void* unit) noexcept = 0;
    virtual NativeNodeStorage* render_root_0c(void* selected_set) noexcept = 0;

    // Complete providers. Concrete defaults perform actual storage operations.
    // ECX=this, RET unless stated. Tokens are not process-callable source code.
    virtual void call_004e6480(void* model) = 0;
    virtual void* call_007103a0(void* list); // 30h circular sentinel; ECX unused
    virtual void* call_007103c0(void* list); // same allocation, distinct native entry
    // Concrete default: full actual group-row producer and source CRT domain.
    virtual void call_00713380(void* model);
    virtual const char* call_unit_10(std::uint32_t entry, void* unit) = 0;
    virtual void call_00b6f960(NativeNodeStorage* render_root,
        const NativeString& name, NativeStringRawPoolContext&); // actual node.name_54; RET4
    virtual void call_00712440(void* model) = 0; // EAX discarded
    virtual void call_00710ad0(void* model) = 0;
    virtual void call_00711c60(void* model); // complete actual part-entry producer

    // Whole storage cleanup defaults include the decoded post-CRT-call tails.
    // Selected-set cleanup remains required. The new C++ unwind domain requires
    // cleanup to return; throwing from these noexcept operations terminates.
    virtual void call_004e6570(void* model) noexcept;
    virtual void call_00711080(void* selected_set_cell) noexcept = 0;
    virtual void call_00712b40(void* groups) noexcept;
    virtual void call_00710f90(void* list) noexcept;
    virtual void call_00710fc0(void* list) noexcept;
    virtual void call_00711000(NativeRenderPointerArrayStorage&) noexcept;
};

// Complete caller 007135C0..00713723 (356 bytes). Original ECX=fresh storage,
// stack(unit, selected set), RET8, EAX=storage. The allocation belongs to the
// caller; failure destroys completed members/base, never frees that allocation.
// Canonical pooled-string bodies are used directly through the actual raw pool.
// This is a new C++ ABI and source exception domain, not native FH3/SEH, Lua
// longjmp, hardware-exception or game-validation evidence.
void* construct_native_unit_part_007135c0(NativeUnitPartConstructionView,
    void* canonical_unit, void* selected_set, NativeStringRawPoolContext&,
    NativeUnitPartConstructionBindings&);

} // namespace bsp
