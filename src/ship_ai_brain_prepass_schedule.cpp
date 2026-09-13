#include "bsp/ship_ai_brain_prepass_schedule.hpp"
#include "bsp/vector_helpers.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Brain pre-pass requires MSVC Win32 x87 operations.
#endif

namespace bsp {
namespace {
void copy_bits(float& target,const volatile float& value) noexcept {
    float* const output_cell=&target;const volatile float* const input_cell=&value;
    __asm {mov eax,input_cell} __asm {movss xmm0,dword ptr[eax]}
    __asm {mov eax,output_cell} __asm {movss dword ptr[eax],xmm0}
}
void copy_x87(float& target,const volatile float& value) noexcept {
    float* const output_cell=&target;const volatile float* const input_cell=&value;
    __asm {mov eax,input_cell} __asm {fld dword ptr[eax]}
    __asm {mov eax,output_cell} __asm {fstp dword ptr[eax]}
}
void difference(float& target,const float& a,const float& b) noexcept {
    float* const output_cell=&target;const float* const left=&a;const float* const right=&b;
    __asm {mov eax,left} __asm {fld dword ptr[eax]}
    __asm {mov eax,right} __asm {fsub dword ptr[eax]}
    __asm {mov eax,output_cell} __asm {fstp dword ptr[eax]}
}
void hull_squared(float& target,const volatile float& length,
    const volatile double& scale) noexcept {
    float spill;float* const output_cell=&target;
    const volatile float* const input_cell=&length;const volatile double* const factor=&scale;
    __asm {
        mov eax,input_cell
        fld dword ptr[eax]
        mov eax,factor
        fmul qword ptr[eax]
        fstp spill
        fld spill
        fmul st(0),st(0)
        mov eax,output_cell
        fstp dword ptr[eax]
    }
}
void horizon(float& target,const float& period,const NavigatorBotParameters& p,
    const volatile double& extra) noexcept {
    float spill;float* const output_cell=&target;const float* const timer=&period;
    const float* const observation=&p.torpedo_observation_2_020;
    const float* const prediction=&p.torpedo_predict_2_014;
    const volatile double* const extra_cell=&extra;
    __asm {
        mov eax,timer
        fld dword ptr[eax]
        fstp spill
        mov eax,observation
        fld dword ptr[eax]
        mov eax,prediction
        fadd dword ptr[eax]
        fld spill
        mov eax,extra_cell
        fadd qword ptr[eax]
        fstp spill
        fadd spill
        mov eax,output_cell
        fstp dword ptr[eax]
    }
}
bool reaches(float range,float time,float hull,float delta_x,float delta_z,
    const std::array<float,2>& relative) noexcept {
    float dynamic_squared,distance_squared,dot;
    const float* const velocity=relative.data();std::uint8_t accepted;
    __asm {
        fld range
        fmul time
        fstp dynamic_squared
        fld dynamic_squared
        fmul st(0),st(0)
        fstp dynamic_squared
        fld delta_z
        fld st(0)
        fld delta_x
        fld st(0)
        fmul st(0),st(0)
        fld st(2)
        fmulp st(3),st(0)
        faddp st(2),st(0)
        fxch
        fstp distance_squared
        fld distance_squared
        fld hull
        fcomip st(0),st(1)
        jbe dynamic_gate
        fstp st(0)
        fstp st(1)
        fstp st(0)
        mov accepted,1
        jmp finished
    dynamic_gate:
        fld dynamic_squared
        fcomip st(0),st(1)
        fstp st(0)
        jbe rejected
        mov eax,velocity
        fmul dword ptr[eax]
        fld dword ptr[eax+4]
        fmulp st(2),st(0)
        faddp st(1),st(0)
        fstp dot
        fldz
        fld dot
        fcomip st(0),st(1)
        fstp st(0)
        seta accepted
        jmp finished
    rejected:
        fstp st(1)
        fstp st(0)
        mov accepted,0
    finished:
    }
    return accepted!=0;
}
class ScanLock {
public:
    explicit ScanLock(TrackedCriticalSection* section):section_(section) {
        if(section_) {EnterCriticalSection(&section_->native);depth()+=1u;}
    }
    ~ScanLock() {if(section_){depth()-=1u;LeaveCriticalSection(&section_->native);}}
    ScanLock(const ScanLock&)=delete;
    ScanLock& operator=(const ScanLock&)=delete;
private:
    volatile std::uint32_t& depth() noexcept {
        return reinterpret_cast<volatile std::uint32_t&>(section_->depth);
    }
    TrackedCriticalSection* const section_;
};
void torpedo_candidates(float& period,ShipAiBrainPrepassHost& host,
    const ShipAiBrainPrepassLiterals& literals) {
    const void* const initial_self=host.current_self_aa8();
    const auto initial=host.unit_fields(initial_self);
    if(!initial.world_valid_c8)host.refresh_pose_00414db0(initial_self);
    const void* const current_self=host.current_self_aa8();
    float hull,origin_x,origin_z,time;
    hull_squared(hull,host.unit_fields(current_self).field_9c8,literals.hull_scale_00ceff98);
    const auto& tuning=host.navigator_parameters(host.unit_level_390(current_self));
    copy_bits(origin_x,initial.world_x_fc);
    copy_bits(origin_z,initial.world_z_104);
    horizon(time,period,tuning,literals.horizon_add_00d7a2b0);
    const void* node=host.world_torpedo_head_0220();
    const std::int32_t count=host.world_torpedo_count_021c();
    if(count<=0)return;
    std::uint32_t remaining=static_cast<std::uint32_t>(count);
    do {
        const void* const candidate=host.node_payload_08(node);
        if(host.shot_vtable_38(candidate)!=0 && host.shot_vtable_2c(candidate)!=0
            && host.unit_byte_5d(candidate)==0
            && host.unit_pointer_4f8(candidate)!=host.current_self_aa8()) {
            const auto other=host.unit_fields(candidate);
            if(!other.world_valid_c8)host.refresh_pose_00414db0(candidate);
            float other_x,other_z,delta_x,delta_z;
            copy_x87(other_x,other.world_x_fc);
            const void* const velocity_self=host.current_self_aa8();
            copy_x87(other_z,other.world_z_104);
            difference(delta_x,origin_x,other_x);
            difference(delta_z,origin_z,other_z);
            std::array<float,3> self_scratch,other_scratch;
            const float* const self_velocity=host.velocity_vtable_34(velocity_self,self_scratch);
            const float* const other_velocity=host.velocity_vtable_34(candidate,other_scratch);
            std::array<float,2> relative;
            difference(relative[0],other_velocity[0],self_velocity[0]);
            difference(relative[1],other_velocity[2],self_velocity[2]);
            // Native duplicates each component through another x87 float spill.
            copy_x87(relative[0],relative[0]);copy_x87(relative[1],relative[1]);
            const float length=length_2d_00414c60(relative);
            if(reaches(length,time,hull,delta_x,delta_z,relative))host.admit_torpedo_009f0ad0(candidate);
        }
        --remaining;
        node=host.node_next_04(node); // live reload, even on final iteration
    }while(remaining!=0);
}
} // namespace

void ship_ai_brain_prepass_after_goal_009f158a(ShipAiBrainPrepassView view,
    float seconds,ShipAiBrainPrepassHost& host,const ShipAiBrainPrepassLiterals& literals) {
    // First timer has the same normal floating state rule as the N second
    // timer. Native retains ST0 step between them; this new ABI passes its
    // same binary32 value again. No callbacks occur between timer operations.
    const bool torpedo_due=ship_ai_neighbour_timer_due_009f15cc(
        view.torpedo_countdown_b48,view.torpedo_period_b44,seconds);
    const bool ship_due=ship_ai_neighbour_timer_due_009f15cc(
        view.ship_countdown_b50,view.ship_period_b4c,seconds);
    if(torpedo_due || ship_due) {
        ScanLock guard(view.lock_04); // capture once; owner slot may later change
        if(torpedo_due)torpedo_candidates(view.torpedo_period_b44,host,literals);
        if(ship_due) {
            ship_ai_walk_neighbour_candidates_009f1856(true,host);
            for(void* current=host.special_head_f89ad8();current;
                current=host.special_next_340(current)) {
                refresh_pose_00414db0(host.special_pose(current));
                (void)host.settings_00424c40();
                (void)host.settings_00424c40();
            }
        }
    } // native decrement/Leave occurs before AC4 lookup and all trailing calls
    if(const void* const helper=host.helper_ac4())host.update_helper_009db8f0(helper,seconds);
    const auto& settings=host.settings_00424c40();
    view.avoidance=ship_ai_avoidance_request_prepass_009f1b7b(host.settings_byte_04(settings)!=0);
}
} // namespace bsp
