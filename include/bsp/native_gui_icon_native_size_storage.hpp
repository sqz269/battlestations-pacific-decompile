#pragma once
#include <cstdint>

namespace bsp {
struct GuiTextureCallbacks;
struct GuiIconStateTextureConstants;

// Ordinary caller-retained private homes; no initialization, owner or status.
// A distinct stable scratch is required for each active invocation. Its backing
// and all Source control/carrier/argument storage are separate from the actual
// texture/output/UV/home/provider-write extents. Unknown escape retains reached
// scratch, inputs, current graph and providers with the caller; no replay or
// cleanup/disposition authority is supplied.
struct NativeGuiIconNativeSizeScratch {
    std::uint32_t width_scale_home;
    std::uint32_t conversion_height_home;
};
static_assert(sizeof(NativeGuiIconNativeSizeScratch) == 8);

// Whole AB17B0..AB1854 data/FP schedule, 165B/51 rows. Native ECX is output,
// EDX is texture, the incoming mutable DWORD home holds UV pointer bits, EAX
// returns captured output and RET4 follows. This six-borrow ordinary Source API
// changes that ABI; private-stack/register/FLAGS/fault equivalence is unproved.
//
// Caller supplies actual writable output8B, live texture/profile table with
// authenticated canonical original-VA RO mapping/lifetime, and a real
// mutable addressable DWORD UV-pointer home and the reached readable UV words.
// Legal output/UV/home aliases retain reached read/store order. The UV pointer
// is read only after the height division, while ST0 is live, then the SAME home
// is overwritten by each rounded and sign-masked difference. The captured UV
// base remains fixed; output0 is stored BEFORE vertical UV reads. No snapshot,
// semantic GuiUvRect, new state/reference lifetime or owner is introduced.
//
// Both calls consume the actual captured profile/target pair through the SAME
// genuine callbacks.dimension_from_captured. Height's CURRENT profile/+4C is
// captured while width ST0 is live; only after its FSTP are captured registers
// stored as Source carriers. There is no later profile/table re-read or fallback
// to width/height, default delegate, codeword-to-callable cast or result filter.
// Existing Game consumer accepts its qualified D61948 canonical+04 domain and
// real B3CE70/B3CE80 targets; OTHER is explicitly Source-rejected there. Captured
// words/count checks are not Native authority, current-generation or lifetime
// proof. Keep the same actual owner/registry/provider graph valid throughout.
//
// constants borrows the SAME live authenticated CE3978 float correction and
// CEC380/CEF1B8 double divisor cells; oneD7A24C is not read. Bind addresses only;
// values are read at their reached x87 instructions. Signed DWORD FILD plus
// negative-only correction, double FDIV and float32 spill epochs are explicit.
// No C++ FP arithmetic, reset/control/RAII restore, catch, cleanup, noexcept or
// Origin policy is added. Ambient FP/private fault/SEH/FH3/NoThrow, emitted code,
// actual raw caller/producer/custody/disposition and activation remain unproved.
void* write_native_gui_icon_native_size_storage_00ab17b0(
    void* actual_output_float2, void* actual_texture,
    volatile std::uint32_t& actual_uv_pointer_home,
    NativeGuiIconNativeSizeScratch& scratch,
    const GuiTextureCallbacks& callbacks,
    const GuiIconStateTextureConstants& constants);
} // namespace bsp
