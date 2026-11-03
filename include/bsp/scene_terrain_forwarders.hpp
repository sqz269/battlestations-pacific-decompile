#pragma once

#include <stdexcept>

#include "bsp/game_hosts_scene_contents.hpp"
#include "bsp/landscape_lua_seg55_argon.hpp"

namespace bsp::game {

// A borrowed view of the Landscape's existing +3D0h terrain, for its three
// height/normal forwarders only. No terrain, samples, origin or pose are copied.
// The three bodies read only height_field_3d0; the other LandscapeState fields
// are not a replacement Landscape and must not be passed to its other methods.
class SceneTerrainForwarders final {
public:
    explicit SceneTerrainForwarders(const SceneTerrainHeightField& terrain)
        : terrain_(terrain) {
        landscape_.height_field_3d0 = &terrain_;
    }
    SceneTerrainForwarders(const SceneTerrainForwarders&) = delete;
    SceneTerrainForwarders& operator=(const SceneTerrainForwarders&) = delete;

    const landscape_lua_seg55_argon::LandscapeState& landscape() const noexcept {
        return landscape_;
    }

private:
    using Vec2 = landscape_lua_seg55_argon::Vec2;
    using Vec3 = landscape_lua_seg55_argon::Vec3;
    using GeneratedModelNode = landscape_lua_seg55_argon::GeneratedModelNode;

    class Terrain final : public landscape_lua_seg55_argon::TerrainHeightFieldInterface {
    public:
        explicit Terrain(const SceneTerrainHeightField& terrain) : terrain_(terrain) {}

        // Evidence: bsp.py show 0087FA20 / 0087FA50 / 0087FB90. Their native
        // __thiscall Landscape methods return ST0 / ST0 / out after dispatching
        // through +3D0h slots 24h / 28h / 34h (RET 8 / 8 / 0Ch). For the loaded
        // terrain vtable 00D5D350 those slots are 00ADA160 / 00ADA900 / 00ADA1C0.
        // Keep the local/world coordinate distinction and all existing PC24 math.
        float local_height_vslot24(float x, float z) override {
            return terrain_.local_height_00ada160(x, z);
        }
        float height_at_vslot28(float x, float z) override {
            return terrain_.height_00ada900(x, z);
        }
        Vec3* local_normal_vslot34(Vec3* out, float x, float z) override {
            float normal[3];
            terrain_.local_normal_00ada1c0(x, z, normal);
            *out = {normal[0], normal[1], normal[2]};
            return out;
        }

        // This view lends only the three proven query slots. Reject accidental
        // use by another Landscape body instead of supplying fabricated state.
        void get_world_bounds_vslot0c(Vec3&, Vec3&) override { unsupported(); }
        Vec2* local_grid_to_world_xz_vslot1c(Vec2*, float, float) override { unsupported(); }
        float height_at_world_vslot2c(float, float) override { unsupported(); }
        bool trace_segment_vslot3c(const void*, const void*, Vec3&) override { unsupported(); }
        GeneratedModelNode* get_terrain_render_root_vslot40() override { unsupported(); }

    private:
        [[noreturn]] static void unsupported() {
            throw std::logic_error("SceneTerrainForwarders only lends terrain slots 24h/28h/34h");
        }
        const SceneTerrainHeightField& terrain_;
    };

    Terrain terrain_;
    landscape_lua_seg55_argon::LandscapeState landscape_;
};

}  // namespace bsp::game
