#pragma once
#include "bsp/native_gui_text_default_widget_lifetime.hpp"
#include "bsp/native_gui_text_main_geometry_storage.hpp"
#include "bsp/native_gui_text_primary_model_storage.hpp"
#include <exception>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Raw Text main-section storage requires MSVC Win32.
#endif

namespace bsp {
// ONE completed post-constructor graph. All records and providers are borrowed,
// nonmoving and independently retained; no Model/NodeBinding/SceneResource copy.
struct NativeGuiTextMainSectionStorageContext final {
    NativeGuiTextDefaultWidgetLifetimeRecord& text;
    NativeGuiTextDefaultWidgetLifetime& authentication;
    const NativeGuiTextPrimaryModelStorageContext& primary_context;
    const NativeGuiTextPrimaryModelStorageFrame& primary_frame;
    const NativeGuiTextPrimaryModelStorageAcquired& primary;
    const NativeGuiTextMainGeometryStorageContext& geometry_context;
    const NativeGuiTextMainGeometryStorageFrame& geometry_frame;
    const NativeGuiTextMainGeometryStorageAcquired& geometry;
    NativeGuiTextRawModelSectionStorageContext& resources;
};

enum class NativeGuiTextMainSectionStoragePhase : std::uint8_t { fresh, entered, complete, failed };
struct NativeGuiTextMainSectionStorageAcquired final {
    using Phase = NativeGuiTextMainSectionStoragePhase;
    enum class Boundary { none, admission, initial188, model, section, append,
        name_construct, material, name_return, material_publication,
        section_release, material_release, parenting, root };
    Phase phase{Phase::fresh};
    Boundary boundary{Boundary::none}, escaped_boundary{Boundary::none};
    std::uint32_t active_call_site{}, escaped_call_site{};
    void* destination{};
    void* captured188{};
    void* first_primary{};
    void* current_primary{};
    void* first_geometry{};
    void* current_geometry{};
    std::uint32_t section_count{};
    bool mesh_gate_read{}, creation_skipped{}, name_construct_entered{}, name_construct_returned{};
    void* captured_name_data{}; // diagnostic only after normal return
    bool name_return_entered{}, name_cleanup_returned{};
    std::int32_t native_eh_state{-1}; // observed normal state only, no EH authority
    NativeGuiTextSectionStorageSection section;
    NativeTextureSurfaceReferenceIncrement pair_decrement{};
    std::uint8_t parenting_flag{}, root_flag{};
    void* parenting_receiver{};
    void* root_receiver{};
    bool parenting_flag_read{}, parenting_entered{}, parenting_returned{};
    bool root_flag_read{}, root_entered{}, root_returned{};
    NativeNodeSetParentAcquired parenting;
    std::exception_ptr failure;
    NativeGuiTextMainSectionStorageAcquired() = default;
    NativeGuiTextMainSectionStorageAcquired(const NativeGuiTextMainSectionStorageAcquired&) = delete;
    NativeGuiTextMainSectionStorageAcquired& operator=(const NativeGuiTextMainSectionStorageAcquired&) = delete;
};

// Installed before effects. Raw name bytes have no initializer: only the real
// 41E870 writes them. Parent operands likewise retain preimages until native
// stores. Neither destructor releases native resources; failed/entered dies hard.
class NativeGuiTextMainSectionStorageFrame final {
public:
    NativeGuiTextMainSectionStorageFrame(NativeGuiTextMainSectionStorageContext&,
        NativeGuiTextMainSectionStorageAcquired&) noexcept;
    ~NativeGuiTextMainSectionStorageFrame() noexcept;
    NativeGuiTextMainSectionStorageFrame(const NativeGuiTextMainSectionStorageFrame&) = delete;
    NativeGuiTextMainSectionStorageFrame& operator=(const NativeGuiTextMainSectionStorageFrame&) = delete;
    NativeGuiTextMainSectionStoragePhase phase() const noexcept { return phase_; }
private:
    friend void ensure_native_gui_text_post_constructor_sections_storage_00ab8530(
        NativeGuiTextRawModelReference&, NativeGuiTextMainSectionStorageContext&,
        NativeGuiTextMainSectionStorageFrame&, NativeGuiTextMainSectionStorageAcquired&);
    NativeGuiTextMainSectionStorageContext& context_;
    NativeGuiTextMainSectionStorageAcquired& acquired_;
    volatile std::uint32_t name_[2];
    NativeNodeParentingFrame parenting_;
    NativeGuiTextMainSectionStoragePhase phase_{NativeGuiTextMainSectionStoragePhase::fresh};
};

// Qualified AB8530 CURRENT-nonnull188 path, main arm AB86CB..AB87B4 ONLY.
// Never replay constructor85 or create another188. Actual late15C must remain
// zero; genuine B6E680(null) and B6D890(null) execute over childless/unparented/
// rootless internal188. Foreign/replaced/nonzero/hierarchy continuations retain
// prior effects and fail. No fake type/parent/scene/layout/stream/Font provider.
//
// Caller owns real GuiDefault/cache/Lua/VFS/sampler/renderer/material readiness,
// raw three-cell/string domains and ALL opaque/transitive extents. Serialize
// publications and exclude receiver/operative Model/mesh retirement during
// remaining reads/callbacks. Keep records, companions, actual objects, providers
// and acquisition backing alive through credits/failures/flags0/terminal return.
// Known concrete extent checks cannot prove transitive provenance/disjointness.
// ALL escapes retain/rethrow; no name/pool/base cleanup, retry or inferred native
// exception origin. Normal name return leaves stale bytes; never rediscover its
// data afterward. Complete/skipped is not another credit or settled custody.
// New C++ library, not original thiscall/private-stack/FH3/SEH/FP ABI, not called
// by ABA8D0/content/prefix owner, and no raw retirement admission or body credit.
void ensure_native_gui_text_post_constructor_sections_storage_00ab8530(
    NativeGuiTextRawModelReference&, NativeGuiTextMainSectionStorageContext&,
    NativeGuiTextMainSectionStorageFrame&, NativeGuiTextMainSectionStorageAcquired&);
} // namespace bsp
