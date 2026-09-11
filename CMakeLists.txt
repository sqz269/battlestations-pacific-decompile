cmake_minimum_required(VERSION 3.21)
project(bsp_reconstruction VERSION 0.1.0 LANGUAGES C CXX)
if(NOT WIN32 OR NOT MSVC OR NOT CMAKE_SIZEOF_VOID_P EQUAL 4)
  message(FATAL_ERROR "Use MSVC targeting Win32 (scripts/build.ps1). The original is x86.")
endif()
include(cmake/lua.cmake)
include(cmake/zlib.cmake)
include(cmake/d3dx.cmake)
add_library(bsp_core STATIC src/math.cpp src/render_sort.cpp src/random.cpp src/random_threads.cpp src/platform_loop.cpp src/win32_event.cpp src/d3d9_startup.cpp src/d3d9_texture.cpp src/texture_atlas.cpp src/texture_atlas_lookup.cpp src/gui_texture.cpp src/gui_geometry.cpp src/shader_source.cpp src/shader_lua.cpp src/material_constants.cpp src/camera_projection.cpp src/camera_inverse.cpp src/camera_multiply.cpp src/camera_affine.cpp src/camera_transform.cpp src/d3d9_states.cpp src/d3d9_resources.cpp src/d3d9_buffers.cpp src/vertex_declaration.cpp src/d3d9_vertex_layout.cpp)
target_include_directories(bsp_core PUBLIC include)
target_sources(bsp_core PRIVATE src/shader_reflection.cpp)
target_sources(bsp_core PRIVATE src/material_samplers.cpp)
target_sources(bsp_core PRIVATE src/material_shadow_samplers.cpp)
target_sources(bsp_core PRIVATE src/material_lighting.cpp)
target_sources(bsp_core PRIVATE src/resource_path.cpp)
target_sources(bsp_core PRIVATE src/vfs_mounts.cpp src/vfs_candidates.cpp
  src/physical_directory.cpp src/vfs_search_defaults.cpp src/resource_lookup.cpp)
target_sources(bsp_core PRIVATE src/vfs_mount_registration.cpp src/package_mounts.cpp)
target_sources(bsp_core PRIVATE src/package_scan.cpp src/vfs_provider_manager.cpp src/vfs_pending.cpp)
target_sources(bsp_core PRIVATE src/file_store.cpp src/file_store_requests.cpp)
target_sources(bsp_core PRIVATE src/mounted_streams.cpp src/resource_preload.cpp)

target_sources(bsp_core PRIVATE src/inflate_stream.cpp)
target_sources(bsp_core PRIVATE src/mpkg_archive.cpp src/mpkg_provider.cpp
  src/resource_enumeration.cpp src/physical_pending_reads.cpp)
target_sources(bsp_core PRIVATE src/frame_clock.cpp)
target_sources(bsp_core PRIVATE src/text_input.cpp)
target_sources(bsp_core PRIVATE src/physical_file.cpp)
target_sources(bsp_core PRIVATE src/memory_stream.cpp)
target_sources(bsp_core PRIVATE src/structured_reader.cpp src/structured_resource.cpp src/structured_hierarchy.cpp)
target_sources(bsp_core PRIVATE src/compiled_material.cpp src/mesh_decode_bindings.cpp src/mesh_gpu_streams.cpp src/building_instance.cpp src/resource_parser_map.cpp src/structured_resource_registry.cpp)
target_sources(bsp_core PRIVATE src/instance_geometry.cpp src/instance_upload.cpp src/instance_grouping.cpp src/structured_model.cpp src/material_clone.cpp src/instance_sort.cpp)
target_sources(bsp_core PRIVATE src/scene_attachment.cpp src/model_bounds.cpp
  src/generated_model_lifetime.cpp src/camera_frame_state.cpp src/material_entry_dispatch.cpp
  src/material_effect_plane.cpp)
target_sources(bsp_core PRIVATE src/material_constant_builder.cpp
  src/material_transform_constants.cpp src/material_scalar_constants.cpp src/material_point_light_constants.cpp)
target_sources(bsp_core PRIVATE src/system_camera_constants.cpp
  src/system_fog_constants.cpp src/system_lighting_constants.cpp
  src/system_camera_axes.cpp src/system_time_constants.cpp src/system_constant_builder.cpp
  src/legacy_crt_math.cpp src/singleton_lifetime.cpp src/particle_clock_lifetime.cpp)
target_sources(bsp_core PRIVATE src/system_fog_owner.cpp src/environment_fog_apply.cpp
  src/authored_fog_source.cpp src/system_lighting_owners.cpp)
target_sources(bsp_core PRIVATE src/native_node_construction.cpp src/light_scene_retention.cpp
  src/system_fog_world_factory.cpp src/allocator_list.cpp src/directional_light_pool.cpp
  src/native_node_destruction.cpp src/light_type_bootstrap.cpp src/directional_light_owner.cpp
  src/lighting_configuration_apply.cpp src/native_shadow_texture_access.cpp
  src/native_viewport_owner.cpp src/system_fog_slot.cpp src/native_camera_pool.cpp
  src/camera_look_at.cpp src/camera_type_bootstrap.cpp src/d3d9_surface_pool.cpp
  src/camera_pose.cpp src/camera_plane_initialization.cpp src/native_camera_owner.cpp
  src/native_resource_support.cpp
  src/native_camera_reference.cpp
  src/native_render_context.cpp
  src/native_renderer_parameters.cpp
  src/d3d9_texture2d_pool.cpp src/native_render_pointer_arrays.cpp
  src/native_render_group_storage.cpp src/native_render_group_lifetime.cpp
  src/native_render_resource_record.cpp src/native_render_resource_alias_nodes.cpp
  src/native_render_alias_checked_ops.cpp
  src/native_render_alias_insertion.cpp
  src/native_model_pool.cpp src/model_type_bootstrap.cpp src/native_model_owner.cpp
  src/native_render_batch_lifetime.cpp src/native_render_batch_reference.cpp
  src/native_render_batch_keys.cpp
  src/native_render_state_leaves.cpp
  src/native_render_queue_rows.cpp src/native_render_pointer_slot_sort.cpp
  src/native_pooled_string_substring.cpp
  src/native_pooled_resource_path.cpp
  src/native_event_owner.cpp
  src/native_render_diagnostic_labels.cpp
  src/native_renderer_synchronization_actual.cpp
  src/native_renderer_binding_getters.cpp
  src/native_renderer_surface_bindings.cpp
  src/native_physical_buffer_owner.cpp
  src/native_tree_out_of_range_exception.cpp
  src/native_hardware_layout_tree.cpp
  src/native_hardware_layout_tree_key.cpp
  src/native_hardware_layout_fields.cpp
  src/native_hardware_layout_tree_insert.cpp
  src/native_hardware_layout_pool_allocate.cpp
  src/native_hardware_layout_construct.cpp
  src/native_hardware_layout_factory.cpp
  src/native_hardware_layout_pool_trim.cpp
  src/native_hardware_layout_pool_lifetime.cpp
  src/native_hardware_layout_pool_static.cpp
  src/native_hardware_layout_tree_lifetime.cpp
  src/native_hardware_layout_tree_static.cpp
  src/native_cube_texture_pool_allocate.cpp
  src/native_cube_texture_pool_trim.cpp
  src/native_cube_texture_pool_lifetime.cpp
  src/native_cube_texture_pool_static.cpp
  src/native_cube_texture_base.cpp
  src/native_cube_texture_owner_array_reserve.cpp
  src/native_cube_texture_owner.cpp
  src/native_volume_texture_owner.cpp
  src/native_hardware_layout_device_release.cpp
  src/native_shader_device_reset.cpp
  src/native_dynamic_buffer_device_release.cpp
  src/native_resource_creation_flags.cpp
  src/native_physical_buffer_access.cpp
  src/native_physical_buffer_lock.cpp
  src/native_logical_buffer_device_save.cpp
  src/native_logical_buffer_device_restore.cpp
  src/native_logical_vertex_pool_return.cpp
  src/native_renderer_vertex_layout_binding.cpp
  src/native_renderer_index_binding.cpp
  src/native_texture_device_reset.cpp
  src/native_renderer_resource_restore.cpp
  src/native_renderer_cached_states.cpp
  src/native_renderer_stream_frequency.cpp
  src/native_shader_owner_texture_release.cpp
  src/native_renderer_texture_binding.cpp
  src/native_frame_target_getters.cpp
  src/native_frame_target_vector.cpp
  src/native_renderer_texture_stage_state.cpp
  src/native_occlusion_query_device_reset.cpp
  src/native_diagnostic_sink_lifetime.cpp
  src/native_hardware_layout_owner.cpp
  src/native_logical_index_owner.cpp
  src/native_retained_memory_owners.cpp
  src/native_renderer_texture_name_notification.cpp
  src/native_logical_texture_named_base.cpp
  src/native_texture_surface_cache_storage.cpp
  src/native_texture_2d_owner.cpp
  src/native_vertex_declaration_owner.cpp
  src/native_render_buffer_unregistration.cpp
  src/native_render_cache_construction.cpp
  src/native_render_resource_record_construction.cpp
  src/native_render_resource_record_array.cpp
  src/native_render_resource_container_removal.cpp
  src/native_render_command_owner.cpp
  src/native_legacy_sbo_string.cpp
  src/native_legacy_exception_owner.cpp src/native_render_queue_access.cpp
  src/native_alias_count_growth.cpp src/native_render_preparation_job_owner.cpp
  src/native_surface_owner.cpp
  src/camera_clip_planes.cpp src/camera_configuration.cpp)
target_sources(bsp_core PRIVATE src/material_sort_metadata.cpp src/render_command_queue.cpp src/render_command_execution.cpp)

target_sources(bsp_core PRIVATE src/mesh_buffers.cpp src/mesh_fields.cpp src/mesh_resource.cpp src/vertex_format.cpp)
target_sources(bsp_core PRIVATE src/stream_scalars.cpp src/font_data.cpp src/d3d9_reset_texture.cpp)
target_sources(bsp_core PRIVATE src/font_registry.cpp src/texture_load_policy.cpp)
target_sources(bsp_core PRIVATE src/d3d9_query.cpp)
target_sources(bsp_core PRIVATE src/font_geometry.cpp src/font_layout.cpp src/font_wrapped_layout.cpp
  src/font_geometry_owner.cpp)
target_sources(bsp_core PRIVATE src/font_shader.cpp src/font_resources.cpp src/material_textures.cpp)
target_sources(bsp_core PRIVATE src/material_parameter_bindings.cpp src/effect_cache.cpp)
target_sources(bsp_core PRIVATE src/win32_window.cpp)
target_compile_features(bsp_core PUBLIC cxx_std_17)
target_compile_options(bsp_core PRIVATE /W4 /WX /fp:strict)
target_link_libraries(bsp_core PUBLIC user32 d3d9 PRIVATE bsp_lua511 bsp_d3dx_headers bsp_zlib121)
add_executable(bsp_d3d9_probe src/d3d9_probe.cpp src/d3d9_shader_probe.cpp src/d3d9_atlas_probe.cpp src/d3d9_material_probe.cpp)
target_sources(bsp_d3d9_probe PRIVATE src/d3d9_alpha_probe.cpp)
target_sources(bsp_d3d9_probe PRIVATE src/d3d9_font_probe.cpp)
target_sources(bsp_d3d9_probe PRIVATE src/d3d9_mesh_probe.cpp)
target_sources(bsp_d3d9_probe PRIVATE src/native_camera_probe.cpp)

target_sources(bsp_d3d9_probe PRIVATE src/inflate_stream_probe.cpp src/mpkg_archive_probe.cpp
  src/resource_preload_probe.cpp src/physical_pending_reads_probe.cpp src/structured_resource_probe.cpp)
target_link_libraries(bsp_d3d9_probe PRIVATE bsp_core)
target_link_libraries(bsp_d3d9_probe PRIVATE bsp_d3dx_headers)
target_compile_options(bsp_d3d9_probe PRIVATE /W4 /WX /fp:strict)
add_executable(bsp_platform_probe src/platform_probe.cpp)
target_link_libraries(bsp_platform_probe PRIVATE bsp_core)
target_compile_options(bsp_platform_probe PRIVATE /W4 /WX /fp:strict)
add_executable(bsp_startup_probe src/startup_probe.cpp)
target_link_libraries(bsp_startup_probe PRIVATE bsp_core)
target_compile_options(bsp_startup_probe PRIVATE /W4 /WX /fp:strict)
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/local/timestamp_reference.hpp")
  target_sources(bsp_startup_probe PRIVATE src/timestamp_reference_probe.cpp)
  target_include_directories(bsp_startup_probe PRIVATE local)
  target_compile_definitions(bsp_startup_probe PRIVATE BSP_HAS_TIMESTAMP_REFERENCE=1)
endif()
include(CTest)
if(BUILD_TESTING)
  add_executable(bsp_math_tests tests/math_tests.cpp)
  target_link_libraries(bsp_math_tests PRIVATE bsp_core)
  target_compile_options(bsp_math_tests PRIVATE /W4 /WX /fp:strict)
  add_test(NAME reconstructed_math COMMAND bsp_math_tests)
  if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/local/seed_reference.hpp")
    add_executable(bsp_native_math_tests tests/native_math_tests.cpp)
    target_include_directories(bsp_native_math_tests PRIVATE local)
    target_link_libraries(bsp_native_math_tests PRIVATE bsp_core)
    target_compile_options(bsp_native_math_tests PRIVATE /W4 /WX /fp:strict)
    add_test(NAME native_math_differential COMMAND bsp_native_math_tests)
  endif()
endif()

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/local/gui_geometry_reference.hpp")
  target_sources(bsp_d3d9_probe PRIVATE src/gui_geometry_reference_probe.cpp)
  target_include_directories(bsp_d3d9_probe PRIVATE local)
  target_compile_definitions(bsp_d3d9_probe PRIVATE BSP_HAS_GUI_REFERENCE=1)
endif()

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/local/camera_reference.hpp")
  target_sources(bsp_d3d9_probe PRIVATE src/camera_reference_probe.cpp)
  target_include_directories(bsp_d3d9_probe PRIVATE local)
  target_compile_definitions(bsp_d3d9_probe PRIVATE BSP_HAS_CAMERA_REFERENCE=1)
endif()
