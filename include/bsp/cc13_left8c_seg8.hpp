#pragma once

// Segment 8 (00491170..004BA200) hard x87 leftovers (cc13-left8c): the
// scene-traffic member update 00496BD0 that the earlier passes skipped.
// docs/CC13_LEFT8C.md lists every function with its evidence and the ones that
// were skipped.
//
// Objects are modelled in their Win32 layout through the byte-offset helpers of
// cc13_left8_seg8.hpp; Left8cHost extends Left8bHost with the calls these bodies
// make that the earlier passes had no hook for. Every hook is invoked at the
// point the image calls the routine named in its comment.

#include <cstdint>

#include "bsp/cc13_left8b_seg8.hpp"

namespace bsp::cc13_left8c {

using cc13_left8::at;
using cc13_left8::obj;
using cc13_left8::Object;

class Left8cHost : public cc13_left8b::Left8bHost {
public:
    // 004887A0 __thiscall(list, &result {container, node}, container, node),
    // RET 0Ch: std::list<int>::erase(iterator); unlinks and frees the node,
    // decrements the size at list+8 and returns the next node in result.
    virtual void int_list_erase_004887a0(Object list, Object result, Object container, Object node) = 0;

    // Debris tick.
    // 00903860 __thiscall(ECX = [game+19CCh], const float3* point, float* height), RET 8, AL.
    virtual bool world_ground_height_at_00903860(Object world, const float* point, float* height) = 0;
    // 0043B9C0 (ECX = const float3* point), AL: the surface probe the splash is skipped on.
    virtual bool surface_probe_0043b9c0(const float* point) = 0;
    // 008685E0 (ECX = &out handle, EDX = scene root [game+19ECh]; stack: template
    // handle by value (consumed), const float3* position, option, tail), RET 10h,
    // EAX = &out.
    virtual Object* point_effect_create_at_position_008685e0(Object* out, Object scene_root, Object consumed_template,
                                                            const float* position, std::uint32_t option,
                                                            std::uint32_t tail) = 0;
    // 004845D0 __thiscall(ECX = handle slot, const handle* source), RET 4: counted assignment.
    virtual void retained_assign_004845d0(Object slot, const Object* source) = 0;
    // 004842C0 __thiscall(ECX = effect, const float3* point), RET 4.
    virtual void point_effect_set_world_point_004842c0(Object effect, const float* point) = 0;
    // 0047C2A0 __fastcall(ECX = float16* out, EDX = const float3* angles), EAX = out.
    virtual float* matrix_build_from_euler_yxz_0047c2a0(float* out, const float* angles) = 0;
    // 00419010 __stdcall(x0, y0, x1, y1, x), RET 14h, ST0.
    virtual float interpolate_clamped_00419010(float x0, float y0, float x1, float y1, float x) = 0;
};

// The scene-traffic member update (path runtime slot 1 0049A360 ends in it).
void scene_traffic_update_members_00496bd0(Left8cHost& host, Object runtime);

// The plane debris tick: slot 1 of the debris spawner vtable 00CE6830 (the
// object 00498E60 constructs and 0049A760 fills). Draws from the shared RNG 00BD2F10.
void plane_debris_tick_0049b1f0(Left8cHost& host, Object spawner, float dt);

}  // namespace bsp::cc13_left8c
