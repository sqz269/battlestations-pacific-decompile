#pragma once

#include "bsp/native_mesh_subset_loading.hpp"

#include <cstdint>

namespace bsp {

// Borrow the actual cells written by the observed CD8390..CD83DE sequence.
// These roles do not define a complete Native descriptor or fixed host layout.
struct NativeCameraResourceInitializerFragmentContext {
    volatile std::uint8_t& guard_01090266;
    volatile std::uint32_t& own_id_01090288;
    volatile std::uint32_t& scene_id_0109028c;
    volatile std::uint32_t& root_id_01090290;
    volatile std::uint32_t& name_address_01090294;

    // Exact original literal-address token 00D6327C. No substitute local
    // string, string-content interpretation, ownership or dereference.
    const std::uint32_t literal_address_00d6327c;

    // Exactly three current Scene words [own, root, name], identical to
    // scene_types.storage().scene_01090210. Never a copied ID preimage.
    volatile std::uint32_t* const scene_01090210;
    NativeMeshResourceTypeIds& scene_types;
    TypeIdCounterLifetime& counter;
};

// New ordinary Source interface for the observed local 79-byte sequence only.
// CD8390 has no recorded function owner; original entry ABI/CRT/EH and whole
// function semantics remain unadmitted. This does not create storage or wire
// an application caller. The caller supplies the correct distinct cells,
// actual literal token and canonical Scene view, with the SAME retained
// counter/root domain and valid lifetimes already used by scene_types.
// Nonzero guard returns without writes. Early guard/name and later partial
// stores remain after a Source dependency exception; no rollback or repair.
void initialize_native_camera_resource_fragment_00cd8390(
    const NativeCameraResourceInitializerFragmentContext&);

} // namespace bsp
