#pragma once

#include <cstdint>

namespace bsp::engine_seg11_35_66_cc13 {

// Native FishSchoolModel vtable 00CFD3A8 +20h, recovered 00708120..007086BD.
// ECX=actual model, EDX unused, stack(context,lod,unused_word,visibility_u32),
// RET10. The last original integer is converted to float before base rendering.
// Family extension because engine_seg11_35_66_cc13 pair is leased by oct18b-nC-ai.
// Existing original-image storage/callee bindings are required; no host wiring.
void __fastcall FishSchoolModel_Render_00708120(
    void* self, void* unused_edx, void* context, float lod,
    std::uint32_t unused_word, std::uint32_t visibility_u32);

}  // namespace bsp::engine_seg11_35_66_cc13
