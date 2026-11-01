#pragma once
#include "bsp/native_texture_surface_getter.hpp"
#include <cstdint>
#include <exception>
#include <memory>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Raw Icon texture apply requires MSVC Win32.
#endif

namespace bsp {
struct NativeGuiIconStorageContext;
struct NativeGuiIconRebuildStorageFrame;
struct NativeGuiIconRebuildStorageContext;
struct GuiTextureCallbacks;
struct GuiIconStateTextureConstants;
class NativeCrtStricmpCallees;
namespace game { class GameNativeReadOnlyData; }

// One caller-owned nonmoving raw80 graph. This address-only view creates no
// Native owner/count/layout or readiness. Concrete backing includes the SAME
// renderer/model/mesh/material/clip/transform services, mutable raw key and
// real layout/factory records. Retain every borrower through normal last-use
// and unknown escape. A fresh reached call must not reuse an entered frame.
struct NativeGuiIconTextureApplyRebuildInvocation {
    NativeGuiIconTextureApplyRebuildInvocation(NativeGuiIconRebuildStorageFrame& f,
        NativeGuiIconRebuildStorageContext& c, std::uint32_t& actual_home) noexcept
        : frame(f), context(c), state_home(actual_home) {}
    virtual ~NativeGuiIconTextureApplyRebuildInvocation() = default;
    NativeGuiIconTextureApplyRebuildInvocation(const NativeGuiIconTextureApplyRebuildInvocation&) = delete;
    NativeGuiIconTextureApplyRebuildInvocation& operator=(const NativeGuiIconTextureApplyRebuildInvocation&) = delete;
    NativeGuiIconRebuildStorageFrame& frame;
    NativeGuiIconRebuildStorageContext& context;
    std::uint32_t& state_home;
};
using MakeNativeGuiIconTextureApplyRebuildInvocation =
    std::unique_ptr<NativeGuiIconTextureApplyRebuildInvocation> (*)(void* provider, void* actual_icon);

// Mandatory genuine SAME service graph; no default provider or borrowed cell
// value is manufactured. Import cells are typed callable cells, not recorded
// Original DWORDs. Bind addresses only: their values are read at3928/3936.
// Full CRT services/locale have their existing actual TLS/library domain.
// Canonical mutable CRT V3 exposes the locale role; no V1/V2 zero fallback.
struct NativeGuiIconTextureApplyStorageContext {
    NativeGuiIconStorageContext& icon;
    const game::GameNativeReadOnlyData& data;
    NativeTextureSurfaceReferenceIncrement const volatile& increment_00ce221c;
    NativeTextureSurfaceReferenceIncrement const volatile& decrement_00ce2220;
    const volatile std::int32_t& locale_changed_0109de1c;
    NativeCrtStricmpCallees& stricmp_callees;
    const GuiTextureCallbacks& dimensions;
    const GuiIconStateTextureConstants& size_constants;
    void* rebuild_provider;
    // Cold metadata preparation only: no Native call, payload/value/target
    // read or Native home seed. Returns a distinct retained actual raw80 graph
    // before the CURRENT profile/target capture. No no-op or semantic rebuild.
    MakeNativeGuiIconTextureApplyRebuildInvocation prepare_rebuild;
};

class NativeGuiIconTextureApplyStorageFrame final {
public:
    enum class Phase { fresh, entered, normal_returned, unknown_source_escape };
    struct Observation {
        Phase phase{Phase::fresh};
        void* actual_icon{};
        const volatile std::uint32_t* argument_home_identity{}; // History, not ownership.
        std::uint32_t active_native_site{}, escaped_native_site{};
        std::uint32_t index{}, restored_delta{};
        void* captured_footer_data{};
        std::uint32_t captured_footer_bytes{};
        bool footer_data_captured{}, footer_disarmed{};
        bool getter_entered{}, getter_returned{}, return_entered{}, return_returned{};
        std::exception_ptr failure;
    };
    // User-provided constructor deliberately initializes NO Native home.
    NativeGuiIconTextureApplyStorageFrame();
    // Metadata destruction only under caller-proven complete last-use/drain.
    // UNKNOWN records and their whole graph must remain outside unwind.
    ~NativeGuiIconTextureApplyStorageFrame();
    NativeGuiIconTextureApplyStorageFrame(const NativeGuiIconTextureApplyStorageFrame&) = delete;
    NativeGuiIconTextureApplyStorageFrame& operator=(const NativeGuiIconTextureApplyStorageFrame&) = delete;
    NativeGuiIconTextureApplyStorageFrame(NativeGuiIconTextureApplyStorageFrame&&) = delete;
    NativeGuiIconTextureApplyStorageFrame& operator=(NativeGuiIconTextureApplyStorageFrame&&) = delete;
    const Observation& observation() const noexcept { return observation_; }
private:
    friend void apply_native_gui_icon_texture_storage_00ab3840(void*,
        volatile std::uint32_t (&)[3], NativeGuiIconTextureApplyStorageFrame&,
        NativeGuiIconTextureApplyStorageContext&);
    struct Match;
    Match& prepare_match();
    Match* matches_{}; // Source metadata only, no Native record pool or cap.
    Observation observation_;
    std::uint32_t delta_spill_10; // Ordinary Native backing; no DMI.
    std::uint32_t native_state_24;
    std::uint32_t footer_homes_[3]; // [captured DATA, size, unused1], no DMI.
};

// Whole NORMAL [00AB3840,00AB39ED), inclusive end00AB39EC,429B/143ops.
// Original ECX Icon138 and by-value [nameLEN,nameDATA,texture]/RET0C become
// this ordinary Source API. All three HOMEs are real mutable ordinary DWORDs
// borrowed via volatile views. Caller retains genuine name-buffer ownership
// at entry; normal footer consumes it even with no states/matches. Postreturn
// header bytes are consumed preimages, not another string owner.
//
// Actual F4/F8 state40 storage/backing/subobjects, full default Icon birth,
// generation and canonical texture/Model/provider custody are caller proof.
// Preserve legal operand aliases; Source controls/records and provider/native
// write extents are disjoint except inherited declared callee aliases. No
// registry/profile/count/phase, Source allocation or return proves that graph.
// Same texture skips only counters; dimensions/flags/current80 still run.
// No UV setter, atlas lookup, load, delayed queue or GUI child traversal occurs.
//
// Raw80/dimensions/CRT retain their qualified Source domains. Unknown Source
// escape records/rethrows and retains remaining debt/frames/providers; no
// Native private FH3 argument cleanup, rollback, retry, count repair, extra
// retain, termination policy or Origin issuer is inferred. No FP reset/C++ FP
// substitute is used. Original ABI/FLAGS/private homes/fault/FH3/SEH/NoThrow/
// unmasked FP/whole object, concrete production binding and runtime stay held.
void apply_native_gui_icon_texture_storage_00ab3840(void* actual_icon138,
    volatile std::uint32_t (&actual_argument_homes)[3],
    NativeGuiIconTextureApplyStorageFrame&, NativeGuiIconTextureApplyStorageContext&);
} // namespace bsp
