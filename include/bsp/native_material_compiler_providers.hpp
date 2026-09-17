#pragma once
#include "bsp/native_material_pass_owner.hpp"
#include "bsp/native_texture_loading_cache.hpp"
#include <cstdint>

namespace bsp {
// Original BE40D0 ECX stream; stack value/output-count; RET8. Captures the
// incoming DWORD in its argument cell and forwards its address, size4, and
// output-count to current+28. Source dispatch supports the actual D691B0
// physical producer and its real BF4F50 WriteFile body. No typed stream.
std::uint32_t write_native_compiler_stream_word_00be40d0(void* actual_stream,
    std::uint32_t value, std::uint32_t* actual_count,
    const volatile std::uint32_t* actual_physical_profile_00d691b0);
// Original BE4460 ECX stream; stack actual8h name/output-count; RET8. Capture
// name length, call current+54, reload name data, call current+28 with the
// captured length. Null data uses the actual109DB64 fallback. Both calls write
// the SAME optional output count; no error branch or count accumulation.
std::uint32_t write_native_compiler_stream_string_00be4460(void* actual_stream,
    const void* actual_name, std::uint32_t* actual_count,
    const volatile std::uint32_t* actual_physical_profile_00d691b0,
    const char* actual_empty_0109db64);
// Complete seven-byte LEA EAX,[ECX+1B18]; RET. No reads or null guard.
const void* __fastcall get_native_compiler_renderer_capabilities_00b1ff50(const void*) noexcept;

struct NativeMaterialCompilerPassConstructionFrame final {
    enum class Phase { fresh, running, complete, failed };
    Phase phase{Phase::fresh};
    std::uint32_t native_site{}, exception_state{0xffffffffu};
    NativeMaterialPassStorage* pass{};
    NativeString temporary;
    bool base_constructed{}, states_registered{}, temporary_live{}, texture_published{};
    std::unique_ptr<NativeTextureCacheAcquired> texture;
    NativeMaterialCompilerPassConstructionFrame() = default;
    ~NativeMaterialCompilerPassConstructionFrame();
    NativeMaterialCompilerPassConstructionFrame(const NativeMaterialCompilerPassConstructionFrame&) = delete;
    NativeMaterialCompilerPassConstructionFrame& operator=(const NativeMaterialCompilerPassConstructionFrame&) = delete;
};
// Distinct complete normal B44B10 overload: genuine numeric D5F0A8+64 selects
// the complete actual B319B0 cache, with persistent child state BEFORE loading
// white.tga. Same base/owners/raw pool; transfer returned owner to84 without an
// extra retain. State0/1 are retained on C++ failure together with the child's
// acquisitions. Native FH3 cleanup is recorded, not substituted for failed
// child retirement. Normal cleanup leaves the actual temporary header stale.
NativeMaterialPassStorage* initialize_native_compiler_pass_00b44b10(void*,
    NativeMaterialPassConstructionAccess&, NativeTextureCacheContext&,
    const volatile std::uint32_t* actual_renderer_profile_00d5f0a8,
    NativeMaterialCompilerPassConstructionFrame&);
// These source interfaces do not reproduce private native stack aliases,
// other stream profiles, arbitrary callback register state or original FH3.
} // namespace bsp
