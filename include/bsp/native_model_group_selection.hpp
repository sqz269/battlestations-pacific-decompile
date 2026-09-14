#pragma once
#include <cstdint>

namespace bsp {
struct NativeNodeStorage;

// Borrow the application's current globals and actual native dispatch domain.
// These are live locations, not copied constants; the access identity is saved
// at entry. No model-group storage, node graph or position is owned here.
struct NativeModelGroupSelectionAccess {
    const volatile float* comparison_00d7a24c;
    const volatile float* comparison_00d7a218;
    const volatile double* displacement_00cf81f0;
    // Actual CRT invalid-parameter route. Native BF6713 can return if its
    // configured handler returns; the original continuation is retained.
    void (__cdecl* invalid_parameter_00bf6713)();
    // ECX actual node; EDX target captured from its CURRENT native vtable+2C;
    // stack(position), RET4. Dispatch that target through its actual binding.
    void (__fastcall* node_virtual2c)(NativeNodeStorage*, std::uint32_t captured_target,
        const float* position);
};

// Complete 00710BB0..00710E00. Native ECX actual owner, stack(selected), RET4;
// this new interface borrows access through EDX. The same owner+16C/+170
// 10h-stride group vector and each group's actual+04/+08 pointer vector are
// used throughout. Preserve signed entry gate, unsigned bounds, checked
// iterator continuations/reloads, UCOMISS unordered behavior, and x87 state.
void __fastcall select_native_model_group_00710bb0(void* actual_owner,
    const NativeModelGroupSelectionAccess*, std::uint32_t selected);
} // namespace bsp
