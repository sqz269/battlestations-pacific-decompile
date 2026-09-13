#pragma once

#include "bsp/native_frame_target_owner.hpp"
#include "bsp/native_renderer_material_state_binding.hpp"
#include "bsp/native_renderer_vertex_binding.hpp"

namespace bsp {

// The same genuine 14h owner and material allocation domain as B27A80/B27B90.
// Borrow two original-token DWORDs at D61A3C: BD30E0/B42330. B5F720 and B455C0
// produce that profile in material pass+1C; a runtime caller carrying it into
// renderer+38 has not been established. The reached current profile is an
// explicit application precondition, not a recovered runtime writer claim.
struct NativeRendererThirdStateBindingContext {
    const NativeRendererMaterialStateBindingContext& actual_material_states;
    const volatile std::uint32_t* actual_third_profile_00d61a3c;
};

// Full B27B00..B27B81 (130 bytes); integrator defined and saved the full body.
// Original ECX renderer, stack incoming owner, RET4. This new fastcall adds a
// fixed borrowed context in EDX. Publish/retain before releasing captured old;
// reload current row base/count after each complete B24510 stage/state/value
// call. Counter+1B98 increments only after returning changed binding work.
// Incoming/current row storage remains valid at each reached native access.
// Reached final-zero profiles may be any of the three material state profiles.
void __fastcall bind_native_renderer_material_third_states_00b27b00(
    void* actual_renderer, const NativeRendererThirdStateBindingContext*,
    NativeMaterialStateOwnerStorage* actual_incoming);

// Fixed borrowed providers for the ten evidenced actual owner profiles. All
// contexts share the application's canonical globals, pools, allocation and
// nested ownership domains. Profile words are original numeric selectors,
// never host callbacks. At least two immutable DWORDs are needed for each
// reached profile (the reused binding APIs impose larger extents themselves).
struct NativeRendererCacheClearContext {
    const NativeRendererThirdStateBindingContext& actual_material_states;
    NativeRendererVertexLayoutBindingContext& actual_layout;
    NativeRendererIndexBindingContext& actual_index;
    NativeRendererVertexBindingContext& actual_vertex;
    NativeRendererTextureBindingContext& actual_texture;
    NativeFrameTargetOwnerContext& actual_frame_target;
    const volatile std::uint32_t* actual_frame_profile_00d5e600;
};

// Complete B241C0..B24457 (664 bytes). ORIGINAL ECX is renderer+34, no stack
// arguments, RET. New C++ interface receives that same actual cache pointer.
// At each of 26 current owner cells: capture, decrement its SAME LONG+04,
// current slot0, full BD30E0 current-profile reread/flags1 raw deleting provider,
// then clear only after return. A null cell receives no owner write. There is
// no parent EH cleanup: a throw preserves the failing cell and later work.
// Clears16 texture validity banks/owners,20 sampler-valid banks,4 stream
// records and the native sparse fields in order; +1938 gamma and last4 texture
// owners remain untouched. Borrow writable cache through+1937 and live actual
// owners/services across callbacks. No replacement cache/refcount/registry.
void clear_native_renderer_binding_cache_00b241c0(void* actual_cache_renderer_plus_34,
    NativeRendererCacheClearContext&);

// Complete within current genuine material D61A2C/34/3C, hardware D62AF4,
// index D61DE0, vertex D61D6C, textures D61948/D61870/D618B0 and frame D5E600
// profiles. Terminal selection is current, irrespective of cell position.
// Unsupported profiles are outside the input domain. Raw throwing deleting
// providers are called directly, without a top-level noexcept companion.
// Existing logical lifetime retains its canonical nested +68/+4C owner domain;
// nonnull +4C's runtime producer/type remains independently unproved. These
// interfaces do not establish original caller/SEH ABI or gameplay equivalence.
} // namespace bsp
