#pragma once
#include "bsp/native_camera_storage_construct.hpp"
#include "bsp/native_gui_camera_store_map.hpp"
#include "bsp/native_gui_scene_storage.hpp"
#include "bsp/native_gui_scene_fields.hpp"
#include "bsp/native_scene_resource_storage.hpp"
#include "bsp/native_light_lifetime.hpp"
#include "bsp/native_platform_focus_owners.hpp"
#include "bsp/system_fog_owner.hpp"
#include "bsp/gui_group_bounds.hpp"
#include <optional>

namespace bsp {
struct NativeGuiSceneAcquisitionContext {
    NativePlatformFocusOwnersContext& manager;
    NativeGuiCameraStoreMapBindings& stores;
    NativeGuiSceneIdentityContext& outer;
    NativeGuiSceneFieldsContext& scene_fields;
    NativeSceneResourceIdentityContext& resource;
    NativeCameraStorageConstructContext& camera;
    NativeDirectionalLightIdentityContext& directional;
    const NativeSystemFogConstants& fog_constants;
    NativeSystemFogStorageContext& fog;
    const GuiGroupBoundsCrtAccess& radius;
    const volatile double& squared_radius_00d7a308;
    const volatile std::uint32_t& sixty_four_00ce7820;
    const char* gui_camera_prefix_00d5cb5c;
    const char* gui_lights_name_00d5cb50;
    const char* gui_directional_name_00d5cb3c;
    // SAME actual owner/import/string/root domains and live one/half cells.
    // Pools represent actual0108FFB0/01090154. Their genuine fixed-owner
    // wrappers compose the existing raw pool entries on these SAME objects.
};
struct NativeGuiSceneAcquisitionFrame {
    // Immutable, disjoint metadata borrowing initialized live DWORD backing.
    // F=original entryESP-9Ch after saved EBP/EDI. locals[0..31]=F10..8F.
    // Header/mask/allocation/color/matrix aliases are one shared array, not
    // overlapping C++ aggregates. Native stores alone change its preimages.
    volatile std::uint32_t* locals;
    volatile std::uint32_t* pushed_words; // three caller argument words
    NativeGuiCameraStoreFindScratch& find;
    NativeGuiCameraStoreCreateFrame& create;
    NativeGuiSceneConstructFrame& scene_construct;
    NativeGuiSceneDestroyFrame& scene_destroy;
    const NativeCameraStorageConstructFrame& camera_construct;
    NativeCameraStorageDestroyFrame& camera_destroy;
    NativeSceneResourceConstructFrame& resource_construct;
    NativeLightLifetimeFrame& directional_destroy;
    NativeCameraDeriveLocalFrame directional_world;
    // Existing aggregate provider frames remain disjoint, with preimages and
    // CURRENT argument cells seeded at native push sites. Borrowed-view callees
    // retain their explicit overlaps. Cross-provider private stack-address
    // coincidence/saved registers/return gaps are not source ABI guarantees;
    // live outer headers and native-owned pointers always use locals directly.
};
struct NativeGuiSceneAcquisitionAcquired {
    enum class Failure { none, native_call, host_admission };
    bool started{}, complete{}, reused_store{}, exception_cleanup_started{};
    bool scene_published{}, store_published{}, camera_root_complete{}, directional_root_complete{};
    bool resource_creator_released{}, viewport_creator_released{}, fog_creator_released{};
    bool directional_descriptor_prepared{};
    bool nested_resource_host_prefix_cleaned{};
    std::int32_t native_eh_state{-1};
    std::uint32_t register_mask{}, active_call_site{}, completed_unwind_actions{};
    Failure failure{Failure::none};
    void* scene_allocation{}; void* scene_result{};
    void* camera_allocation{}; void* camera_result{};
    void* resource_allocation{}; void* resource_result{};
    void* viewport_allocation{}; void* viewport_result{};
    void* fog_allocation{}; void* fog_result{};
    void* directional_allocation{}; void* directional_result{};
    void* last_cleanup_pointer{};
    void* host_admission_owner{};
    NativeGuiSceneConstructAcquired scene_construct;
    NativeGuiSceneLifetimeAcquired scene_destroy;
    NativeCameraStorageConstructAcquired camera_construct;
    NativeCameraStorageAcquired camera_destroy;
    NativeSceneResourceAcquired resource_construct;
    NativeLightLifetimeAcquired directional_destroy;
    NativeGuiCameraStoreAcquired store;
    // Separate same-count metadata, no native stores/credits. This record,
    // contexts and all frames remain address-stable until every admitted owner
    // retires and callbacks quiesce. Dropping a live companion is not cleanup.
    std::optional<NativeGuiSceneReference> scene_reference;
    std::optional<NativeCameraStorageReference> camera_reference;
    std::optional<NativeSceneResourceReference> resource_reference;
    std::optional<NativeDirectionalLightStorageReference> directional_reference;
};

// Complete1467B AC59A0, original ECX actual124h page, RET/no result. Page100
// is an actual8B name,108 descriptor,F0 actual24h store,EC actual24h scene;
// nonnull root4C reached by74==0 must be a genuine actual18Ch Group. Current
// callback-written page/store/owner cells remain valid in the SAME domains.
// No logical GuiSceneOwner/CameraState/SceneResource or fake default callbacks.
void acquire_native_gui_scene_storage_00ac59a0(void* actual_page,
    const NativeGuiSceneAcquisitionFrame&, NativeGuiSceneAcquisitionContext&,
    NativeGuiSceneAcquisitionAcquired&);
// Fresh diagnostics required. Native construction-only cleanup states return
// only their current raw allocation/header, never rollback completed owners or
// publications. Host metadata admission occurs at disarmed native frontiers;
// its failure bypasses native cleanup and retains exact state/credits for caller
// disposition. The existing resource provider's nested ambient-admission failure
// is distinct: its registry/name/base prefix cleanup has ALREADY completed when
// it propagates. Bypass further outer cleanup, retaining state7/GuiLights header,
// current mask/raw3Ch allocation and live ambient creator. Its reported nested
// state2 is historical, not another pending cleanup. Caller disposition must
// not repeat that prefix destruction. Cleanup failures terminate; FH3 excluded.
// Genuine current callable imports/allocator/profile targets, valid accessible
// nonnull successful allocations, prepared recursive dispatch frames, metadata
// quiescence and existing provider domains are caller obligations. Current34
// directional dispatch admits D62FB0/B6E870; other targets fail explicitly.
// No native private-stack/binary ABI/full application/game claim.
} // namespace bsp
