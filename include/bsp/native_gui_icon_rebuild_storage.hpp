#pragma once
#include "bsp/gui_native_geometry.hpp"
#include "bsp/gui_text_native_renderer.hpp"
#include "bsp/native_gui_icon_cropped_quad_storage.hpp"
#include "bsp/native_gui_icon_filter_storage.hpp"
#include "bsp/native_gui_widget_clip_storage.hpp"
#include "bsp/native_gui_widget_transform.hpp"
#include "bsp/native_material_factory.hpp"
#include "bsp/native_mesh_section_layout_storage.hpp"
#include <exception>
#include <optional>

namespace bsp {
struct NativeMaterialEffectCacheContext;
struct NativeMaterialDestructionAccess;
struct NativeMaterialParameterAccess;
struct NativeGuiIconRebuildStorageContext {
    NativeRenderActualOwners& owners;
    GuiNativeGeometryOwners& geometry;
    GuiTextNativeRendererServices& graphics;
    void* const volatile& actual_renderer_00f8d394;
    NativeStringRawPoolContext& strings;
    const volatile std::uint32_t& actual_feature_0109eea4;
    NativeMaterialEffectCacheContext& effects;
    NativeMaterialDestructionAccess& material_destruction;
    // Real existing full BF7FBF comparison is required on every reached
    // both-nonzero shader/name LEN path, not merely equal-length table lookup.
    NativeMaterialParameterAccess& parameters;
    NativeGuiWidgetClipBindings& clip;
    NativeGuiWidgetTransformBindings& transform;
    const NativeMeshSectionLayoutStorageContext& layouts;
    const NativePlatformLoadPublication& platform;
    const GuiTextureCallbacks& textures;
    const GuiIconStateTextureConstants& extent_constants;
    const volatile float& zero_00d7a218;
    const volatile float& minimum_width_00d5c3b8;
    const volatile double& epsilon_00d7a268;
    const volatile float& vertical_scale_00e12fd0;
};

enum class NativeGuiIconRebuildStoragePhase { fresh, entered, complete, failed };
// Retained Source record, NOT a native owner or disposition capability. The
// user-provided constructor prepares address-only RO/service metadata; it does
// not zero the ordinary Native words, sample receiver/renderer/import/flags or
// read widget/Model/name/texture payload. No default Context/owner is created.
// Existing nested factory/clip destructor policies remain inherited; keep this
// record outside unwind through failure instead of destroying a failed graph.
struct NativeGuiIconRebuildStorageFrame final {
    NativeGuiIconRebuildStorageFrame(NativeGuiIconRebuildStorageContext&,
        NativeGuiWidgetTransformScratch& initialized_transform_scratch,
        void* actual_retained_layout_key_home);
    ~NativeGuiIconRebuildStorageFrame();
    NativeGuiIconRebuildStorageFrame(const NativeGuiIconRebuildStorageFrame&) = delete;
    NativeGuiIconRebuildStorageFrame& operator=(const NativeGuiIconRebuildStorageFrame&) = delete;
    NativeGuiIconRebuildStorageFrame(NativeGuiIconRebuildStorageFrame&&) = delete;
    NativeGuiIconRebuildStorageFrame& operator=(NativeGuiIconRebuildStorageFrame&&) = delete;

    NativeGuiIconRebuildStorageContext& context;
    NativeGuiWidgetTransformScratch& transform_scratch;
    void* const layout_key_home;
    const void* simple_color_name;
    const void* default_shader_name;
    const void* untextured_name;
    const void* point_name;
    const void* linear_name;
    const void* parameter_names[4];
    const volatile std::uint32_t* material_profile;
    std::uint32_t native_words[13]; // Native B+10..40; no DMI.
    std::uint32_t native_state_home;
    std::uint32_t resize_preserve_home;
    std::uint32_t resize_request_home;
    std::uint32_t quad_argument_homes[7];
    std::uint32_t layout_mesh_home;
    NativeGuiIconFilterScratch filter;
    NativeGuiIconCroppedQuadStorageFrame quad;
    GuiNativeDeclarationAcquired declaration;
    NativeStreamCloneAcquired stream_creator;
    GuiNativeSectionAcquired section_creator;
    NativeMaterialFactoryAcquired material_creator;
    std::optional<NativeGuiWidgetClipOperation> clip_operation;
    void* layout_acquired_before_insertion{};
    NativeGuiIconRebuildStoragePhase phase{NativeGuiIconRebuildStoragePhase::fresh};
    std::uint32_t native_site{};
    void* actual_icon{};
    void* captured_model{};
    void* captured_stream{};
    void* captured_material{};
    NativeMeshSectionLayoutReferenceOperation saved_decrement{};
    std::exception_ptr failure;
};

// Whole AB3CB0 NORMAL default/current80 caller. ECX Icon and stacked WORD state
// become this ordinary Source ABI over an ACTUAL mutable DWORD argument HOME:
// capture LOWWORD before first provider; later write only LOWBYTE for the new-
// section flag and read that CURRENT BYTE AFTER layout callbacks. Upper bits
// are not a bool/default. No semantic GuiIconWidget/GuiIconRuntime projection.
//
// Caller supplies genuine born/live raw Icon, captured current4C Model and its
// actual raw+180 Mesh family, registered stream/section/material/texture/widget
// companions, constants, string pool/imports and SAME renderer/material/clip/
// transform/layout domains. Registry/address/profile values alone do not prove
// those C++ subobjects, generation, custody or retained captured pointer lives.
// Keep captured states/stream and backing across texture/material callbacks;
// no protective Native retain or automatic disposal is added.
//
// Default targets38=B317E0,5C=B287C0, stream10=B49980/14=B49A80,
// current7C=AB1860 and current50=AA6870 use genuine existing bodies. OTHER
// targets require a separate genuine provider and are outside this Source
// composition; no callable numeric codeword, dummy CurrentCalls or successful
// fallback is installed. Existing layout-context OTHER transport must itself
// be genuine; this function does not manufacture its bindings.
//
// Frame/argument/parameter/quad/layout homes and Source controls must have real
// distinct stable backing, disjoint from provider/native write extents except
// declared header/key/home writes. Preserve the initialized transform scratch
// preimage/holes and valid reached layout-key extents; no zero/cap/default key.
// Real callback header/operand changes remain current at their reached epochs.
// Four fixed parameter copies use exact41DD40 and real BF7680 even count0 with
// SAME feature REF. Existing41E870 literal-name constructor remains its qualified
// Source supplier; no stronger CRT/string/private-home/Original contract follows.
//
// AA9F10 operation is born in this retained record only after actual material
// is known, before parameter registrations; its prepare is address metadata.
// Its actual entry still selects CURRENT ancestry after those callbacks. Saved
// CE2220 is captured after section material binding and reused after layout.
// UNKNOWN catch records/rethrows only: no unwind action, rollback/retry/reset,
// count repair, partial cleanup, Native Origin/NoThrow or new terminal policy.
// Native private FS/FH3/SEH/FLAGS/fault/register/RET/EAX/unmaskedFP and production
// caller/whole CRT/custody/runtime/activation remain unproved; no new credit.
void rebuild_native_gui_icon_geometry_storage_00ab3cb0(void* actual_icon,
    std::uint32_t& actual_state_argument_home, NativeGuiIconRebuildStorageFrame&,
    NativeGuiIconRebuildStorageContext&);
} // namespace bsp