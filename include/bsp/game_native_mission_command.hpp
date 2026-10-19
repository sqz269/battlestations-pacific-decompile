#pragma once
#include "bsp/native_camera_frame_command.hpp"
#include "bsp/native_camera_frustum.hpp"
#include "bsp/native_frame_clock_publication.hpp"
#include "bsp/native_instance_collection.hpp"
#include "bsp/native_instance_geometry.hpp"
#include "bsp/native_instance_group_upload.hpp"
#include "bsp/native_logical_buffer_mapping.hpp"
#include "bsp/native_material_constant_build.hpp"
#include "bsp/native_material_entry_dispatch.hpp"
#include "bsp/native_material_pass_execution.hpp"
#include "bsp/native_render_command_execution.hpp"
#include "bsp/native_render_command_owner.hpp"
#include "bsp/native_render_queue_execution.hpp"
#include "bsp/native_render_scene_submission.hpp"
#include "bsp/native_scene_render_traversal.hpp"
#include "bsp/native_system_constant_gather.hpp"
#include <array>
#include <limits>
#include <memory>
#include <optional>
#include <vector>

namespace bsp::game {
// Metadata for the OFF-default installed-model diagnostic. Every reference
// borrows an already retained canonical owner domain. The caller creates no
// replacement renderer, queue, service, clock, geometry cache or string pool.
// Construct this only under BSP_NATIVE_MISSION_SHIP=1; retain through command
// drain, including an interrupted submission/execution.
struct GameNativeMissionCommandProviders {
    NativeInstanceGeometryAccess& geometry;
    NativeRenderGroupModels& models;
    NativeTracelineRenderServices& render_services;
    const NativeFrameClockPublicationContext& clock;
    void* const volatile& foliage_manager_00f8c274;
    void* const volatile& foliage_00f8c210;
    // Static model paths need no animator. Nonnull animator/skin branches must
    // supply these SAME initialized three type words before execution.
    const volatile std::uint32_t* animator_types_010900fc{};
};
} // namespace bsp::game
