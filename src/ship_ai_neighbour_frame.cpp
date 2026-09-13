#include "bsp/ship_ai_neighbour_frame.hpp"

namespace bsp {
namespace {
// 009F0EFC..009F0F54. Preserve the original two coordinate spills, sum spill,
// ordered epsilon branch, ST0 sqrt boundary and repeated result spill.
__declspec(naked) void __fastcall magnitude(const float*, float*, const CameraAxesCrtAccess*) {
    __asm {
        push ebx
        sub esp,38h
        mov [esp],edx
        mov ebx,[esp+40h]
        movss xmm1,[ecx+4]
        movss xmm0,[ecx]
        movss [esp+18h],xmm1
        fld dword ptr [esp+18h]
        movss [esp+14h],xmm0
        fld dword ptr [esp+14h]
        movss [esp+30h],xmm0
        fmul st(0),st(0)
        movss [esp+34h],xmm1
        fld st(1)
        fmulp st(2),st(0)
        faddp st(1),st(0)
        fstp dword ptr [esp+10h]
        fld qword ptr [kShipAiSectorLengthEpsilon]
        fld dword ptr [esp+10h]
        fcomi st(0),st(1)
        fstp st(1)
        jbe zero_length
        mov ecx,ebx
        call native_crt_sqrt_st0_00bf7030
        fstp dword ptr [esp+10h]
        fld dword ptr [esp+10h]
        fstp dword ptr [esp+10h]
        jmp length_done
    zero_length:
        xorps xmm0,xmm0
        fstp st(0)
        movss [esp+10h],xmm0
    length_done:
        mov eax,[esp]
        movss xmm0,[esp+10h]
        movss [eax],xmm0
        add esp,38h
        pop ebx
        ret 4
    }
}

// 009F0F67..009F0FC3. The class*ratio product is spilled AFTER the COMISS;
// unordered first compare enters the magnitude/floor compare. JBE there keeps
// the original pair. Every scaling product follows the original float spills.
__declspec(naked) void __fastcall floor_velocity(float*,const float*,const float*,const float*) {
    __asm {
        sub esp,38h
        mov [esp],ecx
        movss xmm0,[ecx]
        movss [esp+14h],xmm0
        movss [esp+30h],xmm0
        movss xmm0,[ecx+4]
        movss [esp+18h],xmm0
        movss [esp+34h],xmm0
        movss xmm0,[edx]
        movss [esp+10h],xmm0
        mov eax,[esp+3ch]
        mov edx,[esp+40h]
        fld dword ptr [eax]
        fmul dword ptr [edx]
        movss xmm0,[kShipAiCrossingParallelEpsilon]
        comiss xmm0,[esp+10h]
        fstp dword ptr [esp+20h]
        jbe check_floor
        xorps xmm0,xmm0
        movss [esp+30h],xmm0
        movss [esp+34h],xmm0
        jmp floor_done
    check_floor:
        fld dword ptr [esp+10h]
        fld dword ptr [esp+20h]
        fcomi st(0),st(1)
        jbe keep_pair
        fdivrp st(1),st(0)
        fstp dword ptr [esp+20h]
        fld dword ptr [esp+20h]
        fld st(0)
        fmul dword ptr [esp+14h]
        fstp dword ptr [esp+30h]
        fmul dword ptr [esp+18h]
        fstp dword ptr [esp+34h]
        jmp floor_done
    keep_pair:
        fstp st(1)
        fstp st(0)
    floor_done:
        mov eax,[esp]
        movss xmm0,[esp+30h]
        movss [eax],xmm0
        movss xmm0,[esp+34h]
        movss [eax+4],xmm0
        add esp,38h
        ret 8
    }
}

void scale_store(const float& value,const double& scale,float& destination) noexcept {
    __asm {
        mov eax,value
        fld dword ptr [eax]
        mov eax,scale
        fmul qword ptr [eax]
        mov eax,destination
        fstp dword ptr [eax]
    }
}
void copy_x87(const float& value,float& destination) noexcept {
    __asm {
        mov eax,value
        fld dword ptr [eax]
        mov eax,destination
        fstp dword ptr [eax]
    }
}
bool age(float& lifetime,const float& dt) noexcept {
    float spilled;
    unsigned char survives;
    __asm {
        mov eax,lifetime
        fld dword ptr [eax]
        mov edx,dt
        fsub dword ptr [edx]
        fstp spilled
        fld spilled
        fst dword ptr [eax]
        fldz
        fxch
        fcomip st(0),st(1)
        fstp st(0)
        seta survives
    }
    return survives != 0;
}
// Synchronize the semantic cache only at a native owner+5E read. A null owner
// short-circuits that read; owner_gone is not constructor-produced node state.
bool owner_live(ShipAiObstacleNode& node,ShipAiNeighbourFrameHost& host) {
    if(node.owner==nullptr) return false;
    node.owner_gone_5e=host.owner_gone_5e(node.owner)!=0;
    return !node.owner_gone_5e;
}
} // namespace

ShipAiNeighbourRefreshResult ship_ai_neighbour_frame_refresh_009f0ea0(
    const ShipAiNeighbourFrameView& frame,float dt,ShipAiNeighbourFrameHost& host,
    const CameraAxesCrtAccess& crt) {
    const auto initial=host.self_unit_3fc(); // captured EDI before pose callback
    if(initial.pose_valid_c8==0) host.refresh_pose_00414db0(initial.identity);
    const auto velocity_unit=host.self_unit_3fc(); // reload ECX, retain old EDI
    ShipAiNeighbourAvoidBoxInputs inputs;
    inputs.self_x=initial.world_x_fc;
    inputs.self_z=initial.world_z_104;
    const auto velocity=host.world_velocity_vtable34(velocity_unit.identity);
    std::array<float,2> pair{velocity[0],velocity[2]};
    float speed;
    magnitude(pair.data(),&speed,&crt);

    const auto speed_unit=host.self_unit_3fc();
    const void* const captured_class=speed_unit.class_538; // before settings call
    const auto settings=host.settings_00424c40();
    const float& class_speed=host.class_top_speed_500(captured_class);
    floor_velocity(pair.data(),&speed,&class_speed,&settings.minimum_speed_ratio_1b8);
    inputs.self_velocity_x=pair[0];
    inputs.self_velocity_z=pair[1];

    const auto extent_unit=host.self_unit_3fc();
    const auto entry_count=frame.count_604; // native CMP before both FMUL stores
    scale_store(extent_unit.full_length_9c8,kShipAiNeighbourBoxHalfLengthScale,inputs.self_half_length);
    scale_store(extent_unit.full_beam_9cc,kShipAiNeighbourSpeedFraction,inputs.self_half_beam_unread);
    std::int32_t removed=0;
    if(entry_count>0) {
        auto* cursor=frame.slots_608; // stable inline backing, not a vector iterator
        std::int32_t index=0;
        do {
            auto* const node=*cursor;
            if(!age(node->lifetime_78,dt)) {
                host.destroy_node_0064a610(*node);
                host.free_node_00bf65ac(node);
                ++removed;
            } else {
                if(!owner_live(*node,host)) {
                    node->no_pose_68=true; // 009F110A; do not touch no_arc
                } else {
                    const auto bindings=host.node_bindings(*node);
                    (void)owner_live(*node,host); // near's own entry read
                    (void)ship_ai_neighbour_near_box_refresh_009eae20(*node,bindings.motion,bindings.near_host);

                    const auto initial_party=frame.party_3f0;
                    const auto observed_party=host.owner_party_54(node->owner); // even if initial party<0
                    bool accepted=false;
                    if(initial_party>=0) {
                        const auto filter_unit=host.self_unit_3fc();
                        if(host.director_side_filter_241_0080e160(filter_unit.identity)!=0) {
                            const auto current_settings=host.settings_00424c40();
                            if(current_settings.side_filter_04!=0) {
                                if(observed_party==3) accepted=true;
                                else {
                                    const auto current_party=frame.party_3f0;
                                    accepted=current_party==3 || observed_party==current_party;
                                }
                            }
                        }
                    }
                    inputs.avoidance_accepted=accepted;
                    copy_x87(frame.bounds_min_y_1c0,inputs.self_bounds_min_y);
                    copy_x87(frame.bounds_max_y_1bc,inputs.self_bounds_max_y);
                    (void)owner_live(*node,host); // avoid's own entry read
                    (void)ship_ai_neighbour_avoid_box_refresh_009eafc0(*node,bindings.motion,inputs,bindings.avoid_host);
                }
                if(removed>0) frame.slots_608[index-removed]=node;
            }
            ++cursor;
            ++index;
        } while(index<frame.count_604); // reload after callbacks, as native does
        if(removed>0) frame.count_604-=removed;
    }
    return {removed,frame.count_604};
}
} // namespace bsp
