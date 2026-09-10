#include "bsp/world_construct.hpp"

namespace bsp {

WorldObjectLayout construct_world_object_004cb030() noexcept {
    // 004CB030-004CB0AD. The caller memsets the 4BCh block to zero first, so
    // every field the constructor skips is observed as zero; only the fields
    // the constructor writes are given non-default values here.
    WorldObjectLayout layout{};
    layout.vtable = kWorldVTable;
    layout.field_0c = 0;
    layout.field_10 = 0;
    layout.field_14 = 0;
    layout.slot_count = kWorldSlotCount;
    layout.ready_flag = true; // 004CB098, byte +4ACh = 1
    layout.list_head = 1; // non-null sentinel from 004C3080; the value is opaque
    layout.list_size = 0;
    return layout;
}

MarkerManagerLayout construct_marker_manager_006deca0() noexcept {
    // 006DECA0. Five 12-byte std::list members, each given a self-linked
    // sentinel head and a zero size. The sentinel allocators differ per value
    // type, and each sets one flag byte in the node it returns.
    MarkerManagerLayout layout{};
    layout.vtable = kMarkerManagerVTable;
    layout.lists[0] = MarkerListInit{0x006D8590, 0x1D, true, 0}; // +14h
    layout.lists[1] = MarkerListInit{0x006D85E0, 0x25, true, 0}; // +20h
    layout.lists[2] = MarkerListInit{0x006D8660, 0x11, true, 0}; // +2Ch
    layout.lists[3] = MarkerListInit{0x006D8660, 0x11, true, 0}; // +38h
    layout.lists[4] = MarkerListInit{0x006D8660, 0x11, true, 0}; // +44h
    return layout;
}

bool ocean_failure_is_reported() noexcept {
    // 004DF829-004DF867. 0041DD40 sizes a pooled buffer to 1Bh, 00BF7680
    // copies the literal into it and 00419CC0/00BD1510 hand the block straight
    // back. There is no log call, no store and no branch on the result, and
    // the block is entered whenever game+5FCh is non-null rather than on any
    // ocean outcome. So the message never reaches anything.
    return false;
}

WorldConstructResult run_world_construct(WorldConstructHost& host) {
    WorldConstructResult result{};

    // 004DE63C. A debug budget poke, before anything is allocated.
    if (host.debug_render_flag()) {
        host.set_renderer_budget(0x20000000u);
    }

    // 004DE651. operator new 4BCh, memset 0, constructor 004CB030, then
    // 009037F0(world, 1, 1). The post-construct call runs on the null pointer
    // too in the native code; a host is free to reject that.
    result.world = host.create_world(construct_world_object_004cb030());
    host.world_post_construct(result.world, 1, 1);

    // 004DE6A1 and 004DE73F. Two named nodes; the Operator node is 458h and
    // comes from the node allocator that takes its size in ECX.
    result.scene_root = host.create_scene_node(0x24, kWorldSceneRootName);
    result.operator_node = host.create_operator_node(0x458, kOperatorNodeName);
    host.publish_operator_node(result.operator_node);
    host.set_camera_near_plane(result.operator_node, 0.0f); // 00CE380C

    // 004DE7E4. The child is handed to the node and then released, so the node
    // owns it from here on. This is why BSP_Game_DestroyWorld never touches
    // game+1A00h.
    result.operator_child = host.create_operator_child(0x34);
    host.attach_operator_child(result.operator_node, result.operator_child);
    host.release_ref(result.operator_child);

    host.construct_scene_services(); // 004DCDF0
    host.construct_lighting(); // 004C9EC0, creates game+19F8h

    // 004DE944. The whole configuration block splits here.
    const std::uint32_t record = host.scene_record();
    result.config_source
        = record != 0 ? WorldConfigSource::SceneRecord : WorldConfigSource::Defaults;

    if (record != 0) {
        host.set_world_parameter_from_record(
            kCausticsTextureSourceKey, host.record_field(kSceneRecordCausticsSourceOffset));
    } else {
        host.set_world_parameter(kCausticsTextureSourceKey, kCausticsDayLightValue);
    }
    // Both branches install the same three shore-wave pairs.
    for (std::size_t i = 0; i < kShoreWaveLayerCount; ++i) {
        host.set_world_parameter(kShoreWaveSourceKeys[i], kShoreWaveSourceValues[i]);
    }

    // 004DF421. One constructor, two argument shapes.
    result.ocean_from_scene_record = record != 0;
    if (record != 0) {
        result.ocean_owner = host.create_ocean_owner(
            0x40, result.scene_root, host.record_field(kSceneRecordOceanDescOffset));
        host.ocean_set_light(result.ocean_owner, 0);
        host.ocean_set_vector(kSceneRecordOceanVector0Offset, 0); // 00BBCD90
        host.ocean_set_vector(kSceneRecordOceanVector1Offset, 1); // 00BBCDE0
        host.ocean_set_vector(kSceneRecordOceanVector2Offset, 2); // 00BBCD50
        host.ocean_set_scalar(kSceneRecordOceanScalar0Offset, 0); // 00BBF1A0
        host.ocean_set_scalar(kSceneRecordOceanScalar1Offset, 1); // 00BBF200
        host.set_world_parameter_from_record(
            kWaterTracerColorKey, host.record_field(kSceneRecordWaterTracerColorOffset));
    } else {
        result.ocean_owner
            = host.create_ocean_owner_named(0x40, result.scene_root, kDefaultSkyName);
        host.ocean_set_light(result.ocean_owner, 0);
    }
    host.ocean_set_quality(0); // 004DF689, byte 00F889F4

    // 004DF6A3. Shared by the ocean owner and the Operator node, then released.
    result.atmosphere = host.create_atmosphere(0x94);
    if (result.ocean_owner != 0) {
        host.ocean_set_atmosphere(result.ocean_owner, result.atmosphere);
    }
    host.operator_set_atmosphere(result.operator_node, result.atmosphere);
    host.release_ref(result.atmosphere);
    if (record != 0) {
        for (std::size_t i = 0; i < kSceneRecordFogEntryCount; ++i) {
            host.atmosphere_add_layer(result.atmosphere,
                kSceneRecordFogEntryOffset + i * kSceneRecordFogEntryStride, i);
        }
    }

    // 004DF7B5. The flag is a record byte on the record branch and a hard zero
    // on the other; the scene root is the same on both.
    const std::uint8_t sky_flag
        = record != 0 ? host.record_byte(kSceneRecordSkyFlagOffset) : std::uint8_t{0};
    result.sky = host.create_sky(0xB8, result.scene_root, sky_flag);

    // 004DF810. Guarded by the record, not by any ocean result: the literal is
    // built and dropped, then the sky is configured from the record.
    if (record != 0) {
        host.build_unused_literal(kOceanInitFailedLiteral);
        host.sky_configure(result.sky, result.scene_root);
    }

    // 004DF911. Eight objects, indices 0..7.
    for (std::size_t i = 0; i < kChannelObjectCount; ++i) {
        const std::uint32_t object = host.create_channel_object(kChannelObjectSize, i);
        host.register_channel_object(object);
        ++result.channel_objects;
    }

    // 004DF966 onward.
    for (std::size_t i = 0; i < kTailConstructionCount; ++i) {
        const std::uint32_t object = host.create_tail_object(kTailConstructions[i]);
        if (kTailConstructions[i].game_offset == 0x21D4) {
            result.marker_manager = object;
        }
    }

    host.set_input_context(kWorldInputContext, true); // 004DFAD7
    result.input_context_enabled = true;
    return result;
}

} // namespace bsp
