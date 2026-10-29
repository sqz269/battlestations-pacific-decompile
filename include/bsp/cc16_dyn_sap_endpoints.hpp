#pragma once

#include <cstdint>

namespace bsp::cc16_dyn_sap_endpoints {

// 00C40800..00C408FB: EAX object, ECX three-axis array storage, EDX axis;
// no stack arguments, RET0. Remove upper then lower packed 8-byte entries.
// Native object WORD indices are at +20h/+26h+axis*2; each 12-byte axis
// record has the entry pointer at +4 and count at +8. Endpoint tag low bit
// selects the corresponding WORD field in its actual referenced object.
void remove_endpoints_00c40800(
    void* object, void* axis_arrays, std::uint32_t axis) noexcept;

// 00C40B30..00C40CB9: EAX axis, ECX object, EDX axis-array storage; RET0.
// Publish both sortable float keys using the original ambient FLD/FSTP
// epochs, then stably move lower and upper entries left on unsigned keys.
// Requires one free x87 stack slot and honors actual exception controls.
void update_keys_move_left_00c40b30(
    void* object, void* axis_arrays, std::uint32_t axis) noexcept;

// Required actual raw records/storage; no bounds, null, axis, tag or ring
// validity guards are added. New C++ interfaces are not register-ABI adapters,
// exact fault-state replacements, game-side providers or activation.
}  // namespace bsp::cc16_dyn_sap_endpoints
